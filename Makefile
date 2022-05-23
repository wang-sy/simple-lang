gen_submit:

	mkdir submit
	mkdir -p ./submit/ast
	mkdir -p ./submit/parser
	mkdir -p ./submit/scanner
	mkdir -p ./submit/token
	mkdir -p ./submit/check

	cp ./*.cpp ./submit/
	cp ./*.h ./submit/

	cp ./ast/*.h ./submit/ast/

	cp ./parser/*.cpp ./submit/parser/
	cp ./parser/*.h ./submit/parser/

	cp ./scanner/*.cpp ./submit/scanner/
	cp ./scanner/*.h ./submit/scanner/

	cp ./token/*.cpp ./submit/token/
	cp ./token/*.h ./submit/token/

	cp ./check/*.cpp ./submit/check/
	cp ./check/*.h ./submit/check/

	cp ./Makefile ./submit/

	zip -q -r submit.zip ./submit
	rm -rf ./submit

clean:
	rm -rf ./submit ./testfile.txt ./output.txt ./simple-lang