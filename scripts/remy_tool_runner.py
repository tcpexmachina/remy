import os.path
import subprocess
from warnings import warn

ROOTDIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HLINE2 = "=" * 80 + "\n"

class BaseRemyToolRunner(object):
    """Manages the running of Remy tool."""

    def __init__(self, **kwargs):
        self.command = kwargs.pop('command', self.COMMAND)

        self.default_parameters = dict(self.general_default_parameters)
        for _, key in self.program_parameters:
            if key in kwargs:
                self.default_parameters[key] = kwargs.pop(key)

        if kwargs:
            warn("Unrecognized keyword arguments: {}".format(kwargs))

    def _get_parameters(self, parameters, quiet=False):
        """Returns a dict of parameters, including default parameters.
        Warns if any parameters were unrecognized."""
        recognized = [p[1] for p in self.program_parameters]
        unrecognized = [k for k in parameters if k not in recognized]
        if unrecognized and not quiet:
            warn("Unrecognized parameters: {}".format(unrecognized))
        result = dict(self.default_parameters)
        result.update(parameters)
        return result

    @staticmethod
    def _write_to_file(command, output, outfile=None):
        """Writes the given command and output to a given file."""

        if not outfile:
            return
        if isinstance(outfile, str):
            outfile = open(outfile, "w")
            outfile_was_str = True
        else:
            outfile_was_str = False

        outfile.writelines([
            HLINE2,
            "This was the console output for the command:\n",
            "    " + " ".join(command) + "\n",
            HLINE2,
            "\n"
        ])
        outfile.write(output)

        if outfile_was_str:
            outfile.close()

    def run(self, remyccfilename, parameters={}, outfile=None):
        """Runs the executable with the given parameters and returns the output
        (from both stdout and stderr).

        `remyccfilename` is the name of the RemyCC to test.
        `parameters` is a dict of parameters. If any required parameter is not
            specified, the default for this instance is used.
        If `outfile` is specified, it must either be a file object or a string,
            and the output will be written to it.
        """
        parameters = self._get_parameters(parameters)
        command = [self.command, "if={:s}".format(remyccfilename)]
        command += ["{}={}".format(rroptname, parameters[paramname]) for rroptname, paramname in
                self.program_parameters if paramname in parameters]
        output = subprocess.check_output(command, stderr=subprocess.STDOUT)
        output = output.decode()
        self._write_to_file(command, output, outfile)
        return output


class SenderRunnerRunner(BaseRemyToolRunner):

    general_default_parameters = {
        'nsenders': 2,
        'link_ppt': 1.0,
        'delay': 150.0,
        'mean_on': 5000.0,
        'mean_off': 5000.0,
        'buffer_size': 'inf',
        'sim_time': 1000.0,
    }

    program_parameters = [
    #   (sender-runner option name, Python option name)
        ("sender", "sender"),
        ("nsrc", "nsenders"),
        ("link", "link_ppt"),
        ("rtt", "delay"),
        ("on", "mean_on"),
        ("off", "mean_off"),
        ("buf", "buffer_size"),
        ("time", "sim_time"),
    ]

    COMMAND = os.path.join(ROOTDIR, "src", "sender-runner")

class SenderLoggerRunner(BaseRemyToolRunner):

    general_default_parameters = {
        'nsenders': 2,
        'link_ppt': 1.0,
        'delay': 150.0,
        'buffer_size': 'inf',
        'interval': 1.0,
        'sim_time': 1000.0,
    }

    program_parameters = [
    #   (sender-runner option name, Python option name)
        ("sender", "sender"),
        ("nsrc", "nsenders"),
        ("link", "link_ppt"),
        ("rtt", "delay"),
        ("buf", "buffer_size"),
        ("interval", "interval"),
        ("time", "sim_time"),
        ("of", "datafile"),
        ("sender1on", "sender1_on"),
    ]

    COMMAND = os.path.join(ROOTDIR, "src", "sender-logger")
