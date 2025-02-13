CONFIG += qt

QT += widgets
QT += websockets

QT_VERSION_WARNING = "WARNING: Linking against Qt version lower than 5.6.1 is likely to cause CLI tools video encoding to crash due to I/O but in Qt."

win32 {
	equals(QT_MAJOR_VERSION, 5) {
		equals(QT_MINOR_VERSION, 6):lessThan(QT_PATCH_VERSION, 1)) {
			message($$QT_VERSION_WARNING)
		}
		lessThan(QT_MINOR_VERSION, 6) {
			message($$QT_VERSION_WARNING)
		}
	}
}

equals(QT_MAJOR_VERSION, 6) {
	QT += core5compat

	contains(QMAKE_COMPILER, msvc) {
		QMAKE_LFLAGS += '/entry:mainCRTStartup'
	}
}

HOST_64_BIT = contains(QMAKE_HOST.arch, "x86_64")
TARGET_64_BIT = contains(QMAKE_TARGET.arch, "x86_64")
ARCHITECTURE_64_BIT = $$HOST_64_BIT | $$TARGET_64_BIT

PROJECT_DIRECTORY = ../../vsedit-job-server-watcher
COMMON_DIRECTORY = ../..

TARGET = vsedit-job-server-watcher

CONFIG(debug, debug|release) {

	contains(QMAKE_COMPILER, gcc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = $${COMMON_DIRECTORY}/build/debug-64bit-gcc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-64bit-gcc
		} else {
			DESTDIR = $${COMMON_DIRECTORY}/build/debug-32bit-gcc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-32bit-gcc
		}

		QMAKE_CXXFLAGS += -O0
		QMAKE_CXXFLAGS += -g
		QMAKE_CXXFLAGS += -ggdb3
	}

	contains(QMAKE_COMPILER, msvc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = $${COMMON_DIRECTORY}/build/debug-64bit-msvc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-64bit-msvc
		} else {
			DESTDIR = $${COMMON_DIRECTORY}/build/debug-32bit-msvc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-32bit-msvc
		}
	}

} else {

	contains(QMAKE_COMPILER, gcc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = $${COMMON_DIRECTORY}/build/release-64bit-gcc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-64bit-gcc
		} else {
			DESTDIR = $${COMMON_DIRECTORY}/build/release-32bit-gcc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-32bit-gcc
		}

		QMAKE_CXXFLAGS += -O2
		QMAKE_CXXFLAGS += -fexpensive-optimizations
		QMAKE_CXXFLAGS += -funit-at-a-time
	}

	macx {
		QMAKE_CXXFLAGS -= -fexpensive-optimizations
	}

	contains(QMAKE_COMPILER, msvc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = $${COMMON_DIRECTORY}/build/release-64bit-msvc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-64bit-msvc
		} else {
			DESTDIR = $${COMMON_DIRECTORY}/build/release-32bit-msvc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-32bit-msvc
		}
	}

	DEFINES += NDEBUG

}

E = $$escape_expand(\n\t)

macx {
	INCLUDEPATH += /usr/local/include
	ICON = $${COMMON_DIRECTORY}/resources/vsedit.icns
}

win32 {
	equals(QT_MAJOR_VERSION, 5) {
		QT += winextras
	}

	INCLUDEPATH += 'C:/Program Files/VapourSynth/sdk/include/'

	#DEPLOY_COMMAND = windeployqt
	#DEPLOY_TARGET = $$shell_quote($$shell_path($${DESTDIR}/$${TARGET}.exe))
	#QMAKE_POST_LINK += $${DEPLOY_COMMAND} --no-translations $${DEPLOY_TARGET} $${E}

	if($$ARCHITECTURE_64_BIT) {
		message("x86_64 build")
	} else {
		message("x86 build")
		contains(QMAKE_COMPILER, gcc) {
			QMAKE_LFLAGS += -Wl,--large-address-aware
		}
		contains(QMAKE_COMPILER, msvc) {
			QMAKE_LFLAGS += /LARGEADDRESSAWARE
		}
	}
}

contains(QMAKE_COMPILER, clang) {
	QMAKE_CXXFLAGS += -stdlib=libc++
}

contains(QMAKE_COMPILER, gcc) {
	QMAKE_CXXFLAGS += -std=c++17
	QMAKE_CXXFLAGS += -Wall
	QMAKE_CXXFLAGS += -Wextra
	QMAKE_CXXFLAGS += -Wredundant-decls
	QMAKE_CXXFLAGS += -Wshadow
	#QMAKE_CXXFLAGS += -Weffc++
	QMAKE_CXXFLAGS += -pedantic

	LIBS += -L$$[QT_INSTALL_LIBS]
} else {
	CONFIG += c++17
}

include($${COMMON_DIRECTORY}/pro/common.pri)

TEMPLATE = app

RC_ICONS = $${COMMON_DIRECTORY}/resources/vsedit.ico
QMAKE_TARGET_PRODUCT = 'VapourSynth Editor Job Server Watcher'
QMAKE_TARGET_DESCRIPTION = 'VapourSynth Editor Job Server Watcher'

#SUBDIRS

MOC_DIR = $${PROJECT_DIRECTORY}/generated/moc
UI_DIR = $${PROJECT_DIRECTORY}/generated/ui
RCC_DIR = $${PROJECT_DIRECTORY}/generated/rcc

#DEFINES

#TRANSLATIONS

RESOURCES = $${COMMON_DIRECTORY}/resources/vsedit-job-server-watcher.qrc

FORMS += $${COMMON_DIRECTORY}/common-src/log/styled_log_view_settings_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/jobs/job_edit_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/connect_to_server_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/trusted_clients_addresses_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/watcher_main_window.ui

HEADERS += $${COMMON_DIRECTORY}/common-src/application_instance_file_guard/application_instance_file_guard.h
HEADERS += $${COMMON_DIRECTORY}/common-src/helpers.h
HEADERS += $${COMMON_DIRECTORY}/common-src/ipc_defines.h
HEADERS += $${COMMON_DIRECTORY}/common-src/settings/settings_definitions_core.h
HEADERS += $${COMMON_DIRECTORY}/common-src/settings/settings_definitions.h
HEADERS += $${COMMON_DIRECTORY}/common-src/settings/settings_manager_core.h
HEADERS += $${COMMON_DIRECTORY}/common-src/settings/settings_manager.h
HEADERS += $${COMMON_DIRECTORY}/common-src/log/styled_log_view_core.h
HEADERS += $${COMMON_DIRECTORY}/common-src/log/styled_log_view_structures.h
HEADERS += $${COMMON_DIRECTORY}/common-src/log/log_styles_model.h
HEADERS += $${COMMON_DIRECTORY}/common-src/log/styled_log_view_settings_dialog.h
HEADERS += $${COMMON_DIRECTORY}/common-src/log/styled_log_view.h
HEADERS += $${COMMON_DIRECTORY}/common-src/log/vs_editor_log_definitions.h
HEADERS += $${COMMON_DIRECTORY}/common-src/log/vs_editor_log.h
HEADERS += $${COMMON_DIRECTORY}/common-src/vapoursynth/vs_script_library.h
HEADERS += $${COMMON_DIRECTORY}/common-src/vapoursynth/vs_script_processor_structures.h
HEADERS += $${COMMON_DIRECTORY}/common-src/vapoursynth/vapoursynth_script_processor.h
HEADERS += $${COMMON_DIRECTORY}/common-src/jobs/job_variables.h
HEADERS += $${COMMON_DIRECTORY}/common-src/libp2p/p2p.h
HEADERS += $${COMMON_DIRECTORY}/common-src/libp2p/p2p_api.h
HEADERS += $${COMMON_DIRECTORY}/common-src/libp2p/simd/cpuinfo_x86.h
HEADERS += $${COMMON_DIRECTORY}/common-src/libp2p/simd/p2p_simd.h
HEADERS += $${COMMON_DIRECTORY}/common-src/vapoursynth/vs_pack_rgb.h
HEADERS += $${COMMON_DIRECTORY}/common-src/vapoursynth/vs_gray_frame_prop.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/jobs_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/job_edit_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/job_dependencies_delegate.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/job_state_delegate.h
HEADERS += $${PROJECT_DIRECTORY}/src/connect_to_server_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/trusted_clients_addresses_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/watcher_main_window.h

SOURCES += $${COMMON_DIRECTORY}/common-src/helpers.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/application_instance_file_guard/application_instance_file_guard.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/settings/settings_definitions_core.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/settings/settings_definitions.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/settings/settings_manager_core.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/settings/settings_manager.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/log/styled_log_view_core.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/log/styled_log_view_structures.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/log/log_styles_model.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/log/styled_log_view_settings_dialog.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/log/styled_log_view.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/log/vs_editor_log_definitions.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/log/vs_editor_log.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/vapoursynth/vs_script_library.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/vapoursynth/vs_script_processor_structures.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/vapoursynth/vapoursynth_script_processor.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/jobs/job_variables.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/vapoursynth/vs_pack_rgb.cpp
SOURCES += $${COMMON_DIRECTORY}/common-src/vapoursynth/vs_gray_frame_prop.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/jobs_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/job_edit_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/job_dependencies_delegate.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/job_state_delegate.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/connect_to_server_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/trusted_clients_addresses_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/watcher_main_window.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/main.cpp

# libp2p
SOURCES_P2P += $${COMMON_DIRECTORY}/common-src/libp2p/p2p_api.cpp
SOURCES_P2P += $${COMMON_DIRECTORY}/common-src/libp2p/v210.cpp
SOURCES_P2P += $${COMMON_DIRECTORY}/common-src/libp2p/simd/cpuinfo_x86.cpp
SOURCES_P2P += $${COMMON_DIRECTORY}/common-src/libp2p/simd/p2p_simd.cpp
SOURCES_P2P_SSE41 += $${COMMON_DIRECTORY}/common-src/libp2p/simd/p2p_sse41.cpp

p2p.name = p2p
p2p.input = SOURCES_P2P
p2p.dependency_type = TYPE_C
p2p.variable_out = OBJECTS
p2p.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
p2p.commands = $${QMAKE_CXX} $(CXXFLAGS) -DP2P_SIMD $(INCPATH) -c ${QMAKE_FILE_IN}
contains(QMAKE_COMPILER, msvc) {
	p2p.commands += -Fo${QMAKE_FILE_OUT}
} else {
	p2p.commands += -o ${QMAKE_FILE_OUT}
	p2p.commands += -std=c++14
	p2p.commands += -Wno-missing-field-initializers
}
macx {
	p2p.commands += -Wno-gnu
}
QMAKE_EXTRA_COMPILERS += p2p

p2p_sse41.name = p2p_sse41
p2p_sse41.input = SOURCES_P2P_SSE41
p2p_sse41.dependency_type = TYPE_C
p2p_sse41.variable_out = OBJECTS
p2p_sse41.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
p2p_sse41.commands = $${QMAKE_CXX} $(CXXFLAGS) -DP2P_SIMD $(INCPATH) -c ${QMAKE_FILE_IN}
contains(QMAKE_COMPILER, msvc) {
	p2p_sse41.commands += -Fo${QMAKE_FILE_OUT}
} else {
	p2p_sse41.commands += -msse4.1
	p2p_sse41.commands += -o ${QMAKE_FILE_OUT}
	p2p_sse41.commands += -std=c++14
	p2p_sse41.commands += -Wno-missing-field-initializers
}
macx {
	p2p_sse41.commands += -Wno-gnu
}
QMAKE_EXTRA_COMPILERS += p2p_sse41

include($${COMMON_DIRECTORY}/pro/local_quirks.pri)
