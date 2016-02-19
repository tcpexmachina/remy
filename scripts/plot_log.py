#!/usr/bin/python2
"""Reads and plots the data in a simulation log protocol buffer file.
This script requires Python 2, because protobufs doesn't really work for Python 3 yet."""

import argparse
import os
try:
    import simulationresults_pb2
except ImportError:
    print("Run 'make' in the directory one level above this one before using this script.")
    exit(1)
import matplotlib.pyplot as plt
from senderrunner_runner import SenderRunnerRunner
from matplotlib.animation import FuncAnimation
import utils

DEFAULT_PLOTS_DIR = "log-plots"
LAST_PLOTS_SYMLINK = "last-plots"

def pretty(name):
    return name.split('.')[-1].capitalize().replace("_", " ")


class BaseFigureGenerator(object):
    """Abstract base class to generate figures.
    BasePlotGenerator and BaseAnimationGenerator both derive from this class."""

    title = None
    xlabel = None
    ylabel = None
    figfilename = None
    plotsdir = '.'
    plot_kwargs = {}
    file_extension = None

    def __init__(self, **kwargs):
        self._plotsdir = kwargs.pop('plotsdir', self.plotsdir)
        super(BaseFigureGenerator, self).__init__(**kwargs)

    def get_figfilename(self):
        name = os.path.join(self._plotsdir, self.figfilename)
        if not name.endswith("." + self.file_extension):
            name += "." + self.file_extension
        return name

    def generate(self, run_data):
        """Generates the figure for `run_data`, which should be a
        SimulationRunData instance."""
        raise NotImplementedError("Subclasses must implement generate()")

    @staticmethod
    def get_raw_data(run_data, index, attrname):
        attrnames = attrname.split('.')
        result = [getattr(point.sender_data[index], attrnames[0]) for point in run_data.point]
        for attr in attrnames[1:]:
            result = [getattr(point, attr) for point in result]
        return result

    @staticmethod
    def get_times(run_data):
        return [point.seconds for point in run_data.point]


class BasePlotGenerator(BaseFigureGenerator):
    """Abstract base class to generate plots."""

    legend_location = 'lower right'
    file_extension = 'png'

    def iter_plot_data(self, run_data):
        """Iterates through data to be plotted. The default implementation just
        gives the single element `self.get_plot_data(run_data)`. Subclasses
        that need to plot more than one series should override this method."""
        yield self.get_plot_data(run_data) + (None,)

    def get_plot_data(self, run_data):
        raise NotImplementedError("Subclasses must implement either get_plot_data() or iter_plot_data()")

    def generate(self, run_data):
        """Generates the plot for `run_data`, which should be a
        SimulationRunData instance."""

        print("Generating {}...".format(self.get_figfilename()))
        plt.figure()

        for x, y, label in self.iter_plot_data(run_data):
            plt.plot(x, y, label=label, **self.plot_kwargs)

        plt.title(self.title)
        plt.xlabel(self.xlabel)
        plt.ylabel(self.ylabel)
        if hasattr(self, 'get_xlim'):
            plt.xlim(self.get_xlim(run_data))
        if hasattr(self, 'get_ylim'):
            plt.ylim(self.get_ylim(run_data))
        if len(plt.gca().lines) > 1:
            plt.legend(loc=self.legend_location)
        plt.savefig(self.get_figfilename(), format='png', bbox_inches='tight')
        plt.close()


class BaseAnimationGenerator(BaseFigureGenerator):
    """Abstract base class to generate timed animations."""

    interval = 1
    history = 15
    file_extension = 'mp4'

    plot_kwargs = {'linestyle': 'solid', 'linewidth': 0.25, 'color': (0.75, 0.75, 0.75),
            'marker': '.', 'markersize': 12.0, 'markerfacecolor': 'blue', 'markeredgecolor': 'blue'}

    def __init__(self, **kwargs):
        self._interval = kwargs.pop('interval', self.interval)
        super(BaseAnimationGenerator, self).__init__(**kwargs)

    def _animate(self, i):
        if i < self.history:
            self._line.set_data(self._x[:i], self._y[:i])
        else:
            self._line.set_data(self._x[i-self.history:i], self._y[i-self.history:i])
        self._text.set_text('t = {:.2f}'.format(self._times[i]))

    def generate(self, run_data):
        """Generates the animation for `run_data`, which should be a
        SimulationRunData instance."""

        print("Generating {}...".format(self.get_figfilename()))

        self._fig = plt.figure()
        self._ax = plt.axes()
        self._line = self._ax.plot([], [], **self.plot_kwargs)[0]
        self._text = self._ax.text(0.02, 0.02, '')

        self._ax.set_title(self.title)
        self._ax.set_xlabel(self.xlabel)
        self._ax.set_ylabel(self.ylabel)

        self._times = self.get_times(run_data)
        self._x, self._y = self.get_plot_data(run_data)

        self._ax.set_xlim([0, max(self._x)])
        self._ax.set_ylim([0, max(self._y)])

        anim = FuncAnimation(self._fig, self._animate, frames=len(self._times),
            interval=self._interval)
        anim.save(self.get_figfilename())

    def get_plot_data(self, run_data):
        raise NotImplementedError("Subclasses must implement get_plot_data()")


class TimePlotGenerator(BasePlotGenerator):
    """Abstract base class to generate plots where the x-axis is time."""

    xlabel = "Time (ms)"

    def get_xlim(self, run_data):
        x = self.get_times(run_data)
        return [min(x), max(x)]

    def get_values(self, run_data, index):
        raise NotImplementedError("Subclasses must implement get_values()")

    def iter_plot_data(self, run_data):
        x = self.get_times(run_data)
        for i in range(run_data.config.num_senders):
            y = self.get_values(run_data, i)
            label = "sender {:d}".format(i)
            yield x, y, label


class RawDataTimePlotGenerator(TimePlotGenerator):
    """Generates plots where the x-axis is time and the y-axis is taken directly
    from raw data."""

    def __init__(self, attrname, unit=None, **kwargs):
        self.attrname = attrname
        self.figfilename = attrname

        pretty_name = pretty(attrname)
        self.ylabel = pretty_name
        if unit:
            self.ylabel += " ({})".format(unit)
        self.title = pretty_name

        super(RawDataTimePlotGenerator, self).__init__(**kwargs)

    def get_values(self, run_data, index):
        return self.get_raw_data(run_data, index, self.attrname)


class DifferenceQuotientTimePlotGenerator(TimePlotGenerator):
    """Generates plots where the x-axis is time and the y-axis is of the form:
        y[i] = (b[i] - b[i-1]) / (a[i] - a[i-1])
    where a and b are both raw data."""

    def __init__(self, numerator, denominator, title, unit=None, **kwargs):
        self.numerator_attrname = numerator
        self.denominator_attrname = denominator
        self.figfilename = title.replace(" ", "_")
        self.title = title.capitalize()
        self.ylabel = title.capitalize()
        if unit:
            self.ylabel += " ({})".format(unit)

        super(DifferenceQuotientTimePlotGenerator, self).__init__(**kwargs)

    def get_values(self, run_data, index):
        numerator_values = self.get_raw_data(run_data, index, self.numerator_attrname)
        denominator_values = self.get_raw_data(run_data, index, self.denominator_attrname)
        data = [0.0]
        for i in xrange(1, len(numerator_values)):
            n = numerator_values[i] - numerator_values[i-1]
            d = denominator_values[i] - denominator_values[i-1]
            if n == 0 and d == 0:
                data.append(data[-1])
            elif d == 0:
                data.append(0.0)
            else:
                data.append(n/d)
        return data


class BaseParametricPlotGenerator(BasePlotGenerator):

    plot_kwargs = {'linestyle': 'solid', 'linewidth': 0.25, 'color': (0.75, 0.75, 0.75),
            'marker': '.', 'markersize': 5.0, 'markerfacecolor': 'blue', 'markeredgecolor': 'blue'}


class SenderVersusSenderMixin(object):
    """Provides methods to show the correlation between two senders."""

    def __init__(self, attrname, sender_indices, unit=None, **kwargs):
        self.attrname = attrname
        self.figfilename = "{}_sender{:d}_vs_sender{:d}".format(attrname, sender_indices[0], sender_indices[1])

        pretty_name = pretty(attrname)
        axis_label = pretty_name
        if unit:
            axis_label += " ({})".format(unit)
        self.xlabel = axis_label + " of sender {:d}".format(sender_indices[0])
        self.ylabel = axis_label + " of sender {:d}".format(sender_indices[1])
        self.title = pretty_name + ": sender {:d} vs sender {:d}".format(*sender_indices)
        self.indices = sender_indices

        super(SenderVersusSenderMixin, self).__init__(**kwargs)

    def get_plot_data(self, run_data):
        x = self.get_raw_data(run_data, self.indices[0], self.attrname)
        y = self.get_raw_data(run_data, self.indices[1], self.attrname)
        return x, y


class SingleSenderParametricMixin(object):
    """Provides methods to show the correlation between two variables (for one sender)."""

    def __init__(self, attrnames, sender_index, units=None, **kwargs):
        self.attrnames = attrnames
        self.figfilename = "{}_vs_{}_sender{:d}".format(attrnames[0], attrnames[1], sender_index)

        pretty_names = [pretty(attrname) for attrname in attrnames]
        axis_labels = pretty_names
        if units:
            axis_labels = [label + " ({})".format(unit) for label, unit in zip(axis_labels, units)]
        self.xlabel = axis_labels[0]
        self.ylabel = axis_labels[1]
        self.title = "{} vs {} for sender {:d}".format(pretty_names[0], pretty_names[1], sender_index)
        self.index = sender_index

        super(SingleSenderParametricMixin, self).__init__(**kwargs)

    def get_plot_data(self, run_data):
        x = self.get_raw_data(run_data, self.index, self.attrnames[0])
        y = self.get_raw_data(run_data, self.index, self.attrnames[1])
        return x, y


class SenderVersusSenderPlotGenerator(SenderVersusSenderMixin, BaseParametricPlotGenerator):
    """Generates plots that show the correlation between two senders."""
    pass


class SenderVersusSenderAnimationGenerator(SenderVersusSenderMixin, BaseAnimationGenerator):
    """Generates animations that show the progress of two senders with time."""
    pass


class SingleSenderParametricPlotGenerator(SingleSenderParametricMixin, BaseParametricPlotGenerator):
    """Generates plots that show the correlation between two variables."""
    pass


class SingleSenderParametricAnimationGenerator(SingleSenderParametricMixin, BaseAnimationGenerator):
    """Generates animations that show the progress of two variables with time."""
    pass


def make_plots_dir(dirname, argvalue):
    if argvalue.endswith(".data"):
        basename = argvalue[:-5]
    else:
        basename = argvalue
    return utils.make_output_dir(dirname, DEFAULT_PLOTS_DIR, basename, LAST_PLOTS_SYMLINK)

def read_data_file(logfilename):
    logfile = open(logfilename, 'rb')
    data = simulationresults_pb2.SimulationsData()
    data.ParseFromString(logfile.read())
    logfile.close()
    return data

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("inputfile", type=str,
    help="Input file (data file or RemyCC)")
senderrunner_group = parser.add_argument_group("sender-runner arguments", "Only used when --remycc is used")
senderrunner_group.add_argument("-s", "--nsenders", type=int, default=2,
    help="Number of senders")
senderrunner_group.add_argument("-p", "--link-ppt", type=float, default=3.1622776601683795,
    help="Link speed (packets per millisecond)")
senderrunner_group.add_argument("-t", "--delay", type=float, default=150.0,
    help="Delay (milliseconds)")
senderrunner_group.add_argument("-q", "--mean-on", type=float, default=1000.0,
    help="Mean on duration (milliseconds)")
senderrunner_group.add_argument("-w", "--mean-off", type=float, default=1000.0,
    help="Mean off duration (milliseconds)")
senderrunner_group.add_argument("-b", "--buffer-size", type=str, default="inf",
    help="Buffer size, a number or 'inf' for infinite buffers")
senderrunner_group.add_argument("-i", "--interval", type=float, default=0.1,
    help="Logging interval (seconds)")
senderrunner_group.add_argument("-T", "--sim-time", type=float, default=100,
    help="Simulation time to run for (seconds)")
parser.add_argument("-O", "--plots-dir", type=str, default=None,
    help="Directory to place output files in.")
args = parser.parse_args()

# First, try reading it as a data file
data = read_data_file(args.inputfile)

# If there's nothing in it, it was probably a RemyCC
if not data.run_data:
    parameter_keys = ["nsenders", "link_ppt", "delay", "mean_on", "mean_off", "buffer_size", "interval", "sim_time"]
    parameters = {key: getattr(args, key) for key in parameter_keys}
    datafile = args.inputfile + ".data"
    print("Running sender-runner to produce " + datafile)
    runner = SenderRunnerRunner(**parameters)
    runner.run(args.inputfile, datafile=datafile)
    data = read_data_file(datafile)

plotsdir = make_plots_dir(args.plots_dir, args.inputfile)
BaseFigureGenerator.plotsdir = plotsdir
utils.log_arguments(plotsdir, args)

BaseAnimationGenerator.interval = data.settings.log_interval_ticks

generators = [
    RawDataTimePlotGenerator("average_throughput"),
    RawDataTimePlotGenerator("average_delay", "ms"),
    RawDataTimePlotGenerator("sending_duration", "ms"),
    RawDataTimePlotGenerator("packets_received"),
    RawDataTimePlotGenerator("total_delay", "ms"),
    RawDataTimePlotGenerator("window_size"),
    RawDataTimePlotGenerator("intersend_time", "ms"),
    RawDataTimePlotGenerator("memory.rec_send_ewma", "ms"),
    RawDataTimePlotGenerator("memory.rec_rec_ewma", "ms"),
    RawDataTimePlotGenerator("memory.rtt_ratio", "ms"),
    RawDataTimePlotGenerator("memory.slow_rec_rec_ewma", "ms"),
    DifferenceQuotientTimePlotGenerator("packets_received", "sending_duration", "throughput"),
    DifferenceQuotientTimePlotGenerator("total_delay", "packets_received", "delay"),
    SenderVersusSenderPlotGenerator("window_size", (0, 1)),
    SenderVersusSenderPlotGenerator("intersend_time", (0, 1)),
    SenderVersusSenderPlotGenerator("memory.rec_send_ewma", (0, 1)),
    SenderVersusSenderPlotGenerator("memory.rec_rec_ewma", (0, 1)),
    SenderVersusSenderPlotGenerator("memory.rtt_ratio", (0, 1)),
    SenderVersusSenderPlotGenerator("memory.slow_rec_rec_ewma", (0, 1)),
    SingleSenderParametricPlotGenerator(("window_size", "intersend_time"), 0),
    SingleSenderParametricPlotGenerator(("window_size", "intersend_time"), 1),
    SingleSenderParametricPlotGenerator(("memory.rec_send_ewma", "memory.rec_rec_ewma"), 0),
    SingleSenderParametricPlotGenerator(("memory.rec_send_ewma", "memory.rec_rec_ewma"), 1),
    SenderVersusSenderAnimationGenerator("window_size", (0, 1)),
    SenderVersusSenderAnimationGenerator("intersend_time", (0, 1)),
    SenderVersusSenderAnimationGenerator("memory.rec_send_ewma", (0, 1)),
    SenderVersusSenderAnimationGenerator("memory.rec_rec_ewma", (0, 1)),
    SenderVersusSenderAnimationGenerator("memory.rtt_ratio", (0, 1)),
    SenderVersusSenderAnimationGenerator("memory.slow_rec_rec_ewma", (0, 1)),
    SingleSenderParametricAnimationGenerator(("window_size", "intersend_time"), 0),
    SingleSenderParametricAnimationGenerator(("window_size", "intersend_time"), 1),
    SingleSenderParametricAnimationGenerator(("memory.rec_send_ewma", "memory.rec_rec_ewma"), 0),
    SingleSenderParametricAnimationGenerator(("memory.rec_send_ewma", "memory.rec_rec_ewma"), 1),
]

for run_data in data.run_data:
    for generator in generators:
        generator.generate(run_data)
