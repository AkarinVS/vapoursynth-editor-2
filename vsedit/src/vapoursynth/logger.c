#include <stddef.h>
#include <vapoursynth/VapourSynth.h>

static void VS_CC loggerLog(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	int n = vsapi->propNumElements(in, "name");
	for (int i = 0; i < n; i++) {
			const char *msg = vsapi->propGetData(in, "name", i, NULL);
			vsapi->logMessage(mtWarning, msg);
	}
}

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
	configFunc("com.vsedit.logger", "vsedit", "VapourSynth Logger", VAPOURSYNTH_API_VERSION, 1, plugin);
	registerFunc("Logger", "name:data[]:empty", loggerLog, 0, plugin);
}
