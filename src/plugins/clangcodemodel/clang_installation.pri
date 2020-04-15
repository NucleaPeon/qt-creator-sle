isEmpty(LLVM_INSTALL_DIR):LLVM_INSTALL_DIR=$$(LLVM_INSTALL_DIR)
LLVM_INSTALL_DIR ~= s,\\\\,/,g

DEFINES += CLANG_COMPLETION
DEFINES += CLANG_HIGHLIGHTING
#DEFINES += CLANG_INDEXING

defineReplace(findLLVMConfig) {
    LLVM_CONFIG_VARIANTS = \
        llvm-config llvm-config-3.2 llvm-config-3.3 llvm-config-3.4 \
        llvm-config-3.5 llvm-config-3.6 llvm-config-4.0 llvm-config-4.1

    # Prefer llvm-config* from LLVM_INSTALL_DIR
    !isEmpty(LLVM_INSTALL_DIR) {
        for(variant, LLVM_CONFIG_VARIANTS) {
            variant=$$LLVM_INSTALL_DIR/bin/$$variant
            exists($$variant) {
                return($$variant)
            }
        }
    }

    # Find llvm-config* in PATH
    ENV_PATH = $$(PATH)
    ENV_PATH = $$split(ENV_PATH, $$QMAKE_DIRLIST_SEP)
    for(variant, LLVM_CONFIG_VARIANTS) {
        for(path, ENV_PATH) {
            subvariant = $$path/$$variant
            exists($$subvariant) {
                return($$subvariant)
            }
        }
    }

    # Fallback
    return(llvm-config)
}

win32 {
    LLVM_INCLUDEPATH = $$LLVM_INSTALL_DIR/include
    exists ($${LLVM_INSTALL_DIR}/lib/clang.*) {
        CLANG_LIB = clang
    } else {
        exists ($${LLVM_INSTALL_DIR}/lib/libclang.*) {
            CLANG_LIB = libclang
        } else {
            error("Cannot find Clang shared library!")
        }
    }
    LLVM_LIBDIR = $$LLVM_INSTALL_DIR/lib
    LLVM_LIBS = \
        -L$$LLVM_INSTALL_DIR/bin \
        -L$$LLVM_LIBDIR \
        -l$${CLANG_LIB}
    LLVM_LIBS += -ladvapi32 -lshell32
    LLVM_VERSION = 3.4
}

unix {
    LLVM_CONFIG = $$findLLVMConfig()

    LLVM_VERSION = $$system($$LLVM_CONFIG --version)
    LLVM_VERSION = $$replace(LLVM_VERSION, ^(\\d+\\.\\d+).*$, \\1)
    message("... version $$LLVM_VERSION")

    LLVM_INCLUDEPATH = $$system($$LLVM_CONFIG --includedir)
    isEmpty(LLVM_INCLUDEPATH):LLVM_INCLUDEPATH=$$LLVM_INSTALL_DIR/include
    LLVM_LIBDIR = $$system($$LLVM_CONFIG --libdir)
    isEmpty(LLVM_LIBDIR):LLVM_LIBDIR=$$LLVM_INSTALL_DIR/lib

    exists ($${LLVM_LIBDIR}/libclang.*) {
        #message("LLVM was build with autotools")
        CLANG_LIB = clang
    } else {
        exists ($${LLVM_LIBDIR}/liblibclang.*) {
            #message("LLVM was build with CMake")
            CLANG_LIB = libclang
        } else {
            exists ($${LLVM_INSTALL_DIR}/lib/libclang.*) {
                #message("libclang placed separately from LLVM")
                CLANG_LIB = clang
                LLVM_LIBDIR = $${LLVM_INSTALL_DIR}/lib
                LLVM_INCLUDEPATH=$${LLVM_INSTALL_DIR}/include
            } else {
                error("Cannot find Clang shared library!")
            }
        }
    }

    LLVM_LIBS = -L$${LLVM_LIBDIR}
    LLVM_LIBS += -l$${CLANG_LIB}
}

DEFINES += CLANG_VERSION=\\\"$${LLVM_VERSION}\\\"
DEFINES += "\"CLANG_RESOURCE_DIR=\\\"$${LLVM_LIBDIR}/clang/$${LLVM_VERSION}/include\\\"\""
