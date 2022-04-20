import os
import shutil


def my_diff(file_a, file_b):
    f_a = open(file_a, "r")
    f_b = open(file_b, "r")

    lines_a = f_a.readlines()
    lines_b = f_b.readlines()

    if len(lines_a) != len(lines_b):
        print("line_size_neq!!!")
        return

    for i in range(len(lines_a)):
        if lines_a[i] != lines_b[i]:
            print("for line_id = {}, neq!\nline_a: {}\nline_b:{}".
                  format(i + 1, lines_a[i], lines_b[i]))

    f_a.close()
    f_b.close()

    print("no_diff, pass!!!!!")


def run_test_point(base_dir, test_dir_id, test_point_id):
    input_file_path = "{}/{}/testfile{}.txt".format(base_dir, test_dir_id, test_point_id)
    shutil.copy(input_file_path, "testfile.txt")

    os.system('./cmake-build-debug/simple_lang')

    stdout_file = "{}/{}/output{}.txt".format(base_dir, test_dir_id, test_point_id)
    myout_file = "output.txt"

    print("------------------------------")
    print("TestPoint::{}-{}:".format(test_dir_id, test_point_id))
    my_diff(file_a=stdout_file, file_b=myout_file)
    print("==============================\n\n")


def run_lexical_test():
    for i in range(1, 7):
        for j in range(1, 11):
            run_test_point("testdata/lexical", i, j)


if __name__ == "__main__":
    run_lexical_test()
