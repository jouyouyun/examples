TARGET := test_plugin
PLUGIN_TARGET := dell7590.so

build: build-plugin
	go build -o ${TARGET} main.go


build-plugin:
	cd plugin && go build  -buildmode=plugin -o ${PLUGIN_TARGET} && cd ..

clean:
	rm -f plugin/${PLUGIN_TARGET}
	rm -f ${TARGET}

rebuild: clean build
