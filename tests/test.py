import os
import yaml
import subprocess

tests_files = [f.split('.')[0] for f in os.listdir("tests") if os.path.isfile(os.path.join("tests", f))]
PATH_42SH = "./../src/42sh"
PATH_OUTPUT = ".output.out"
PATH_ERR = ".output.err"
stderr_name = "stderr/stderr.txt"
stdout_name = "stderr/stdout.txt"
nb_test = 0
success = 0


def run_test(test, i, tests_length):

    branch = "├── " if i < tests_length - 1 else "└── "
    test_command = test["command"] if "command" in test else None
    test_script = test["script"] if "script" in test else None

    command = PATH_42SH
    if (test_command):
        command += " -c " + f"\'{test_command}\'"
    else:
        command += f" \'scripts/{test_script}\'"

    expected_status = test["status"]
    expected_output = test["output"].strip()

    output = ""
    suc = 0
    try:
        with open(PATH_ERR, "w+") as file_err:
            with open(PATH_OUTPUT, "w+") as file_out:
                actual_status = subprocess.run(command, shell=True, stdout=file_out, stderr=file_err).returncode

        with open(PATH_OUTPUT, "r") as file_out:
            actual_output = file_out.read().strip()

        with open(PATH_ERR, "r") as file_err:
            actual_err = file_err.read().strip()

        if actual_status == expected_status:
            if actual_output == expected_output:
                if actual_status == 0 and actual_err != "":
                    output, suc = branch + "\033[91m✗ " + test["name"] + f" \033[0m Expecting nothing on stderr, got: {actual_err}\033[0m", 0
                elif actual_status != 0 and actual_err == "" and test["name"].find("Exit") == -1:
                    output, suc = branch + "\033[91m✗ " + test["name"] + f" \033[0m Expecting something on stderr, got nothing. \033[0m", 0
                else:
                    output, suc = branch + "\033[92m✓ " + test["name"] + "\033[0m", 1
            else:
                output, suc = branch + "\033[91m✗ " + test["name"] + f"\033[0m Expected: {expected_output}, got: {actual_output}\033[0m", 0
        else:
            output, suc = branch + "\033[91m✗ " + test["name"] + f"\033[0m Expected: {expected_status}, got: {actual_status}, err: {actual_err}\033[0m", 0

    except Exception as e:
        print(f"Error executing test '{test['name']}': {e}")

    return output, suc

def run_tests():
    global tests_files, nb_test, success

    buff = ""
    for tests in tests_files:
        print("\033[93m" + tests + "\033[0m")
        with open("tests/" + tests + ".yml", "r") as file:
            tests_data = yaml.safe_load(file)["tests"]

        output = []
        test_success = 0
        test_nb_tests = 0
        for i,test in enumerate(tests_data):
            out, suc = run_test(test, i, len(tests_data))
            output.append(out)
            test_nb_tests += 1
            test_success += suc

        nb_test += test_nb_tests
        success += test_success

        if (test_nb_tests == test_success):
            buff += "\033[92m✓ " + f"{tests}" + "\n\033[0m"
        else:
            buff += "\033[91m✗ " + f"{tests}" + "\n\033[0m"
        buff += "\n".join(output) + "\n\n"


    print("Final Test over")
    ratio = success / nb_test * 100
    length = len("{:.2f}%".format(ratio)) + 21
    print("┌" + "─" * length + "┐")
    if not success == nb_test:
        print("│   CHECK RESULT : " + "{:.2f}%".format(ratio) + "   │")
    else:
        print("│   CHECK RESULT : " + "{:.2f}%".format(ratio) + "   │")
    print("└" + "─" * length + "┘" + "\n")

    print(buff)

if __name__ == "__main__":
    run_tests()
