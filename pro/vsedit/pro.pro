CONFIG += qt

QT += widgets

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

HOST_64_BIT = contains(QMAKE_HOST.arch, "x86_64")
TARGET_64_BIT = contains(QMAKE_TARGET.arch, "x86_64")
ARCHITECTURE_64_BIT = $$HOST_64_BIT | $$TARGET_64_BIT

PROJECT_DIRECTORY = ../../vsedit
COMMON_DIRECTORY = ../..

CONFIG(debug, debug|release) {

	contains(QMAKE_COMPILER, gcc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = $${COMMON_DIRECTORY}/build/debug-64bit-gcc
			TARGET = vsedit-debug-64bit-gcc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-64bit-gcc
		} else {
			DESTDIR = $${COMMON_DIRECTORY}/build/debug-32bit-gcc
			TARGET = vsedit-debug-32bit-gcc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-32bit-gcc
		}

		QMAKE_CXXFLAGS += -O0
		QMAKE_CXXFLAGS += -g
		QMAKE_CXXFLAGS += -ggdb3
	}

	contains(QMAKE_COMPILER, msvc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = $${COMMON_DIRECTORY}/build/debug-64bit-msvc
			TARGET = vsedit-debug-64bit-msvc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-64bit-msvc
		} else {
			DESTDIR = $${COMMON_DIRECTORY}/build/debug-32bit-msvc
			TARGET = vsedit-debug-32bit-msvc
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-debug-32bit-msvc
		}
	}

} else {

	contains(QMAKE_COMPILER, gcc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = $${COMMON_DIRECTORY}/build/release-64bit-gcc
			TARGET = vsedit
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-64bit-gcc
		} else {
			DESTDIR = $${COMMON_DIRECTORY}/build/release-32bit-gcc
			TARGET = vsedit-32bit
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-32bit-gcc
		}

		QMAKE_CXXFLAGS += -O2
		QMAKE_CXXFLAGS += -fexpensive-optimizations
		QMAKE_CXXFLAGS += -funit-at-a-time
	}

	contains(QMAKE_COMPILER, msvc) {
		if($$ARCHITECTURE_64_BIT) {
			DESTDIR = $${COMMON_DIRECTORY}/build/release-64bit-msvc
			TARGET = vsedit
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-64bit-msvc
		} else {
			DESTDIR = $${COMMON_DIRECTORY}/build/release-32bit-msvc
			TARGET = vsedit-32bit
			OBJECTS_DIR = $${PROJECT_DIRECTORY}/generated/obj-release-32bit-msvc
		}
	}

	DEFINES += NDEBUG

}

S = $${DIR_SEPARATOR}

D = $$DESTDIR
D = $$replace(D, $$escape_expand(\\), $$S)
D = $$replace(D, /, $$S)

SC = $${COMMON_DIRECTORY}/
SC = $$replace(SC, $$escape_expand(\\), $$S)
SC = $$replace(SC, /, $$S)

E = $$escape_expand(\n\t)

QMAKE_POST_LINK += $${QMAKE_COPY} $${SC}$${S}resources$${S}vsedit.ico $${D}$${S}vsedit.ico $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} $${SC}$${S}resources$${S}vsedit.svg $${D}$${S}vsedit.svg $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} $${SC}$${S}README $${D}$${S}README $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} $${SC}$${S}LICENSE $${D}$${S}LICENSE $${E}
QMAKE_POST_LINK += $${QMAKE_COPY} $${SC}$${S}CHANGELOG $${D}$${S}CHANGELOG $${E}

macx {
	INCLUDEPATH += /usr/local/include
	ICON = $${COMMON_DIRECTORY}/resources/vsedit.icns
}

win32 {
	QT += winextras

	INCLUDEPATH += 'C:/Program Files (x86)/VapourSynth/sdk/include/'

	DEPLOY_COMMAND = windeployqt
	DEPLOY_TARGET = $$shell_quote($$shell_path($${D}/$${TARGET}.exe))
	QMAKE_POST_LINK += $${DEPLOY_COMMAND} --no-translations $${DEPLOY_TARGET} $${E}

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
	QMAKE_CXXFLAGS += -std=c++11
	QMAKE_CXXFLAGS += -Wall
	QMAKE_CXXFLAGS += -Wextra
	QMAKE_CXXFLAGS += -Wredundant-decls
	QMAKE_CXXFLAGS += -Wshadow
	#QMAKE_CXXFLAGS += -Weffc++
	QMAKE_CXXFLAGS += -pedantic

	LIBS += -L$$[QT_INSTALL_LIBS]
} else {
	CONFIG += c++11
}

TEMPLATE = app

VER_MAJ = 16
VERSION = $$VER_MAJ

RC_ICONS = $${COMMON_DIRECTORY}/resources/vsedit.ico
QMAKE_TARGET_PRODUCT = 'VapourSynth Editor'
QMAKE_TARGET_COMPANY = 'Aleksey [Mystery Keeper] Lyashin'
QMAKE_TARGET_COPYRIGHT = $$QMAKE_TARGET_COMPANY
QMAKE_TARGET_DESCRIPTION = 'VapourSynth script editor'

#SUBDIRS

MOC_DIR = $${PROJECT_DIRECTORY}/generated/moc
UI_DIR = $${PROJECT_DIRECTORY}/generated/ui
RCC_DIR = $${PROJECT_DIRECTORY}/generated/rcc

#DEFINES

#TRANSLATIONS

RESOURCES = $${COMMON_DIRECTORY}/resources/vsedit.qrc

FORMS += $${PROJECT_DIRECTORY}/src/settings/settings_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/log/styled_log_view_settings_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/script_status_bar_widget/script_status_bar_widget.ui
FORMS += $${PROJECT_DIRECTORY}/src/preview/preview_advanced_settings_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/preview/preview_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/frame_consumers/benchmark_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/script_templates/templates_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/jobs/job_edit_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/jobs/jobs_dialog.ui
FORMS += $${PROJECT_DIRECTORY}/src/main_window.ui

HEADERS += $${PROJECT_DIRECTORY}/src/common/helpers.h
HEADERS += $${PROJECT_DIRECTORY}/src/common/aligned_vector.h
HEADERS += $${PROJECT_DIRECTORY}/src/common/chrono.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/settings_definitions.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/settings_manager.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/actions_hotkey_edit_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/clearable_key_sequence_editor.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/item_delegate_for_hotkey.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/theme_elements_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/settings/settings_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/log/styled_log_view_structures.h
HEADERS += $${PROJECT_DIRECTORY}/src/log/log_styles_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/log/styled_log_view_settings_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/log/styled_log_view.h
HEADERS += $${PROJECT_DIRECTORY}/src/log/vs_editor_log.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_status_bar_widget/script_status_bar_widget.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/scroll_navigator.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/preview_area.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/timeline_slider.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/preview_advanced_settings_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/preview/preview_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_editor/number_matcher.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_editor/syntax_highlighter.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_editor/script_completer_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_editor/script_completer.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_editor/script_editor.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_plugin_data.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vapoursynth_plugins_manager.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_library.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_processor_structures.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vapoursynth_script_processor.h
HEADERS += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_processor_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/frame_consumers/benchmark_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/frame_header_writers/frame_header_writer.h
HEADERS += $${PROJECT_DIRECTORY}/src/frame_header_writers/frame_header_writer_null.h
HEADERS += $${PROJECT_DIRECTORY}/src/frame_header_writers/frame_header_writer_y4m.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_templates/drop_file_category_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/script_templates/templates_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/job.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/job_definitions.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/jobs_model.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/job_state_delegate.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/job_dependencies_delegate.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/job_edit_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/jobs/jobs_dialog.h
HEADERS += $${PROJECT_DIRECTORY}/src/main_window.h

SOURCES += $${PROJECT_DIRECTORY}/src/common/helpers.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/settings_definitions.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/settings_manager.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/actions_hotkey_edit_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/clearable_key_sequence_editor.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/item_delegate_for_hotkey.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/theme_elements_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/settings/settings_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/log/styled_log_view_structures.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/log/log_styles_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/log/styled_log_view_settings_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/log/styled_log_view.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/log/vs_editor_log.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_status_bar_widget/script_status_bar_widget.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/scroll_navigator.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/preview_area.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/timeline_slider.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/preview_advanced_settings_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/preview/preview_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_editor/number_matcher.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_editor/syntax_highlighter.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_editor/script_completer_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_editor/script_completer.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_editor/script_editor.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_plugin_data.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vapoursynth_plugins_manager.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_library.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_processor_structures.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vapoursynth_script_processor.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/vapoursynth/vs_script_processor_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/frame_consumers/benchmark_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/frame_header_writers/frame_header_writer.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/frame_header_writers/frame_header_writer_null.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/frame_header_writers/frame_header_writer_y4m.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_templates/drop_file_category_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/script_templates/templates_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/job.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/jobs_model.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/job_state_delegate.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/job_dependencies_delegate.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/job_edit_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/jobs/jobs_dialog.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/main_window.cpp
SOURCES += $${PROJECT_DIRECTORY}/src/main.cpp

include($${COMMON_DIRECTORY}/pro/local_quirks.pri)
