package filetype

// #cgo pkg-config: libmagic
// #include <stdlib.h>
// #include "filetype.h"
import "C"
import "unsafe"

func QueryFiletype(filename string) string {
	var cFilename = C.CString(filename)
	defer C.free(unsafe.Pointer(cFilename))
	var cFiletype = C.query_filetype(cFilename)
	defer C.free(unsafe.Pointer(cFiletype))

	var filetype = C.GoString(cFiletype)
	if len(filetype) == 0 {
		return ""
	}

	return filetype
}

func IsExecutableFile(filename string) bool {
	var filetype = QueryFiletype(filename)
	if len(filename) == 0 {
		return false
	}

	var execTypeList = []string{
		"application/x-binary",
		"application/x-elf",
		"application/x-executable",
		"application/x-object",
		"application/x-sharedlib",
		"application/x-pie-executable",
		"text/x-java",
		"application/x-java",
		"application/x-java-jnilib",
		"application/x-java-jnlp-file",
		"application/x-java-pack200",
		"application/x-java-vm",
		"application/java-archive",
		"application/octet-stream",
		"application/x-msdownload",
		"text/javascript",
		"application/javascript",
		"application/x-javascript",
		"text/x-sh",
		"text/x-script.sh",
		"application/x-sh",
		"text/x-shellscript",
		"text/x-script.phyton",
		"application/x-bytecode.python",
		"text/x-tcl",
		"application/x-tcl",
		"text/aspdotnet",
		"text/x-clojure",
		"text/x-coffeescript",
		"application/x-lisp",
		"text/x-script.lisp",
		"text/x-emacs-lisp",
		"text/x-erlang",
		"text/x-lua",
		" text/x-php",
		"text/x-python",
		"text/x-ruby",
		"text/x-sql",
		"application/x-dosexec",
	}

	for i := 0; i < len(execTypeList); i++ {
		if filetype == execTypeList[i] {
			return true
		}
	}
	return false
}
