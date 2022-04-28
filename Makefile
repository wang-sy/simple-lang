gen_submit: clean
	mkdir submit
	cp -r `find . | fgrep ".cpp"` ./submit
	cp -r `find . | fgrep ".h"` ./submit
	rm -rf ./submit/CMakeCXXCompilerId.cpp

clean:
	rm -rf ./submit ./testfile.txt ./output.txt ./simple-lang