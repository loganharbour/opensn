import os
import subprocess
import re


class TestSlot:
    """Data structure to hold information regarding the parallel execution
       of a test"""

    def __init__(self, test, argv):
        self.process = None
        self.test = test
        self.passed = False
        self.argv = argv
        self.command: str = ""

        self._Run()

    def _Run(self):
        """Protected method to actually run the test"""
        test = self.test
        self.test.submitted = True

        if test.skip != "":
            return

        cmd = self.argv.mpi_cmd + " " + str(test.num_procs) + " "
        cmd += self.argv.exe + " "
        cmd += test.filename + " "
        cmd += "--suppress-color "
        cmd += "master_export=false "
        for arg in test.args:
            if arg.find("\"") >= 0:
                cmd += "'" + arg + "' "
            else:
                cmd += arg + " "
        self.command = cmd

        # cmd += f"> out/{test.filename}.out "
        # cmd += "2>&1 "
        self.process = subprocess.Popen(cmd,
                                        cwd=test.file_dir,
                                        shell=True,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE,
                                        universal_newlines=True)
        # test_path = os.path.relpath(test.file_dir + test.filename)
        # print("Submitting test " + test_path)

    def Probe(self):
        """Probes the test to see if it finished"""
        running = True
        test = self.test

        if test.ran:
            running = False
            return running

        if not test.ran and test.skip != "":
            self.PerformChecks()
            test.ran = True
            running = False
            return running

        if self.process.poll() is not None:

            out, err = self.process.communicate()

            file = open(test.file_dir + f"out/{test.GetOutFilenamePrefix()}.out", "w")
            file.write(self.command + "\n")
            file.write(out + "\n")
            file.write(err + "\n")
            file.close()

            if not self.test.ran:
                self.PerformChecks()

            self.test.ran = True
            # print("done with " + test.GetTestPath())

            running = False

        return running

    def PerformChecks(self):
        """Applies to check-suite for the test"""
        test = self.test
        passed = True
        output_filename = f"{test.file_dir}out/{test.GetOutFilenamePrefix()}.out"

        if test.skip == "":
            error_code = self.process.returncode
            for check in self.test.checks:
                verbose = self.argv.verbose
                check_passed = check.PerformCheck(output_filename,
                                                  error_code, verbose)
                passed = passed and check_passed

                check_annotations = check.GetAnnotations()
                for ann in check_annotations:
                    test.annotations.append(ann)
        else:
            test.annotations.append("skipped")

        test_path = os.path.join(test.file_dir, test.filename)
        test_file_name = os.path.relpath(test_path, self.argv.directory)

        if not os.path.isfile(test_path):
            test.annotations.append("lua file missing")

        pad = 0
        if passed:
            self.passed = True
            message = "\033[32mPassed\033[0m"
            pad += 5 + 4
        else:
            self.passed = False
            message = "\033[31mFailed\033[0m"
            pad += 5 + 4

        prefix = "\033[33m[{:2d}]\033[0m".format(test.num_procs)
        pad += 5 + 4

        for annotation in test.annotations:
            message = f"\033[36m[{annotation}]\033[0m" + message
            pad += 5 + 4

        width = 120 - len(prefix + test_file_name) + pad
        message = message.rjust(width, ".")

        opensn_elapsed_time_sec = 0.0
        if os.path.exists(output_filename):
          for line in open(output_filename, 'r'):
            found = re.search("Elapsed execution time:", line)
            if found:
              values_slice = re.split(r'[,:]', line.strip())
              opensn_elapsed_time_sec = float(values_slice[1])*3600 + float(values_slice[2])*60 + float(values_slice[3])
              break

        time_taken_message = " {:.1f}s".format(opensn_elapsed_time_sec)

        print(test_path + message + time_taken_message)

        if test.skip != "":
            print("Skip reason: " + test.skip)
