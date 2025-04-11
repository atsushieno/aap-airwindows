#include <aap/android-audio-plugin.h>
#include <aap/ext/parameters.h>
#include <aap/ext/state.h>
#include <aap/ext/presets.h>
#include <aap/ext/plugin-info.h>
#include <aap/ext/midi.h>
#include "AirwinRegistry.h"
#include "cmidi2.h"

class AAPAirwindowsPluginContext {
public:
    AndroidAudioPluginFactory *pluginFactory;
    std::string pluginUniqueID;
    AndroidAudioPluginHost *host;
    AirwinRegistry::awReg& registry;
    std::map<int32_t,float> parameter_default_values{};
    bool active{false};
    std::unique_ptr<AirwinConsolidatedBase> instance{nullptr};
    std::vector<float*> audioInputs{};
    std::vector<float*> audioOutputs{};

    AAPAirwindowsPluginContext(
        AndroidAudioPluginFactory *pluginFactory,
        const char *pluginUniqueID,
        int sampleRate,
        AndroidAudioPluginHost *host,
        AirwinRegistry::awReg& registry) :
        pluginFactory(pluginFactory),
        pluginUniqueID(pluginUniqueID),
        host(host),
        registry(registry) {
        instance = registry.generator();
        instance->setSampleRate((float) sampleRate);

        // Store the initial parameter values as defaults, so that we can fill
        // valid default values later at anytime.
        for (size_t i = 0, n = registry.nParams; i < n; i++)
            parameter_default_values[i] = instance->getParameter(i);
    }
};

//----

int32_t aap_airwindows_get_parameter_count(aap_parameters_extension_t* ext, AndroidAudioPlugin *plugin) {
    auto ctx = (AAPAirwindowsPluginContext *) plugin->plugin_specific;
    return ctx->registry.nParams;
}

aap_parameter_info_t aap_airwindows_get_parameter(aap_parameters_extension_t* ext, AndroidAudioPlugin *plugin, int32_t index) {
    auto ctx = (AAPAirwindowsPluginContext *) plugin->plugin_specific;
    aap_parameter_info_t info;
    info.stable_id = index;
    ctx->instance->getParameterName(index, info.display_name);
    info.path[0] = '\0';
    info.min_value = 0;
    info.max_value = 1;
    info.default_value = ctx->parameter_default_values[index];
    info.per_note_enabled = false;
    return info;
}

double aap_airwindows_get_parameter_property(aap_parameters_extension_t* ext, AndroidAudioPlugin *plugin, int32_t parameterId, int32_t propertyId) {
    return 0;
}

int32_t aap_airwindows_get_enumeration_count(aap_parameters_extension_t* ext, AndroidAudioPlugin *plugin, int32_t parameterId) {
    // We cannot support unknown number of display names...
    return 0;
}

aap_parameter_enum_t aap_airwindows_get_enumeration(aap_parameters_extension_t* ext, AndroidAudioPlugin *plugin, int32_t parameterId, int32_t enumIndex) {
    aap_parameter_enum_t empty;
    return empty;
}

aap_parameters_extension_t params_ext{
    nullptr,
    aap_airwindows_get_parameter_count,
    aap_airwindows_get_parameter,
    aap_airwindows_get_parameter_property,
    aap_airwindows_get_enumeration_count,
    aap_airwindows_get_enumeration
};

aap_state_extension_t state_ext;

//----

void aap_airwindows_prepare(
        struct AndroidAudioPlugin *plugin,
        aap_buffer_t* audioBuffer) {

}

void aap_airwindows_activate(struct AndroidAudioPlugin *plugin) {
    auto ctx = (AAPAirwindowsPluginContext *) plugin->plugin_specific;
    ctx->active = true;
}

void aap_airwindows_deactivate(struct AndroidAudioPlugin *plugin) {
    auto ctx = (AAPAirwindowsPluginContext *) plugin->plugin_specific;
    ctx->active = false;
}

void aap_airwindows_process(
        struct AndroidAudioPlugin *plugin,
        aap_buffer_t* audioBuffer,
        int32_t frameCount,
        int64_t timeoutInNanoseconds) {
    auto ctx = (AAPAirwindowsPluginContext *) plugin->plugin_specific;
    auto instance = ctx->instance.get();
    auto info = plugin->get_plugin_info(plugin);
    size_t nPorts = info.get_port_count(&info);
    std::vector<float*>& inputs = ctx->audioInputs;
    std::vector<float*>& outputs = ctx->audioOutputs;
    for (int i = 0; i < nPorts; i++) {
        auto port = info.get_port(&info, i);
        switch (port.content_type(&port)) {
            case AAP_CONTENT_TYPE_AUDIO: {
                auto buf = (float *) audioBuffer->get_buffer(*audioBuffer, i);
                if (port.direction(&port) == AAP_PORT_DIRECTION_INPUT) {
                    inputs.emplace_back(buf);
                } else {
                    outputs.emplace_back(buf);
                }
                break;
            }
            case AAP_CONTENT_TYPE_MIDI2: {
                auto mbh = (AAPMidiBufferHeader *) audioBuffer->get_buffer(*audioBuffer, i);
                if (port.direction(&port) == AAP_PORT_DIRECTION_INPUT) {

                    CMIDI2_UMP_SEQUENCE_FOREACH((uint8_t*) mbh + sizeof(AAPMidiBufferHeader), mbh->length, iter) {
                        auto ump = (cmidi2_ump*) iter;
                        switch (cmidi2_ump_get_message_type(ump)) {
                            case CMIDI2_MESSAGE_TYPE_MIDI_2_CHANNEL: {
                                switch (cmidi2_ump_get_status_code(ump)) {
                                    case CMIDI2_STATUS_NRPN: {
                                        auto index = cmidi2_ump_get_midi2_nrpn_msb(ump) * 0x100 + cmidi2_ump_get_midi2_nrpn_lsb(ump);
                                        auto data = cmidi2_ump_get_midi2_nrpn_data(ump);
                                        instance->setParameter(index, static_cast<float>(data / (double) UINT32_MAX));
                                        break;
                                    }
                                }
                                break;
                            }
                            case CMIDI2_MESSAGE_TYPE_SYSEX8_MDS: {
                                // FIXME: verify if it is indeed AAP param sysex8

                                uint8_t group{}, channel{}, key{}, extra{};
                                uint16_t index{};
                                float value{};
                                aapReadMidi2ParameterSysex8(&group, &channel, &key, &extra,
                                                            &index, &value,
                                                            ump[0], ump[1], ump[2], ump[3]);
                                instance->setParameter(index, value);
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    instance->processReplacing(inputs.data(), outputs.data(), frameCount);
}

void* aap_airwindows_get_extension(
        struct AndroidAudioPlugin *plugin,
        const char *uri) {
    if (strcmp(uri, AAP_PARAMETERS_EXTENSION_URI) == 0) {
        return &params_ext;
    }
    if (strcmp(uri, AAP_STATE_EXTENSION_URI) == 0) {
        return &state_ext;
    }
    return nullptr;
}

aap_plugin_info_t aap_airwindows_get_plugin_info(
        struct AndroidAudioPlugin* plugin) {
    auto ctx = (AAPAirwindowsPluginContext *) plugin->plugin_specific;
    auto hostExt = (aap_host_plugin_info_extension_t*) ctx->host->get_extension(ctx->host, AAP_PLUGIN_INFO_EXTENSION_URI);
    return hostExt->get(hostExt, ctx->host, ctx->pluginUniqueID.c_str());
}

//----

AndroidAudioPlugin *aap_airwindows_plugin_new(
        AndroidAudioPluginFactory *pluginFactory,
        const char *pluginUniqueID,
        int sampleRate,
        AndroidAudioPluginHost *host) {
    const char* urnPrefix = "urn:airwindows:";
    const char* urnPart = strstr(pluginUniqueID, urnPrefix);
    if (urnPart == nullptr) {
        // FIXME: log error maybe?
        return nullptr; // invalid plugin name
    }
    urnPart += strlen(urnPrefix);
    auto nameEnd = strchr(urnPart, ':');
    char name[1024];
    size_t nameSize = nameEnd - urnPart;
    strncpy(name, urnPart, nameSize);
    if (nameSize < 1023)
        name[nameSize] = '\0';

    auto index = AirwinRegistry::nameToIndex[name];
    auto& reg = AirwinRegistry::registry[index];

    auto ctx = new AAPAirwindowsPluginContext(pluginFactory, pluginUniqueID, sampleRate, host, reg);
    auto ret = new AndroidAudioPlugin {
            ctx,
            aap_airwindows_prepare,
            aap_airwindows_activate,
            aap_airwindows_process,
            aap_airwindows_deactivate,
            aap_airwindows_get_extension,
            aap_airwindows_get_plugin_info,
    };
    return ret;
}

void aap_airwindows_plugin_delete(AndroidAudioPluginFactory *, AndroidAudioPlugin *plugin) {
    auto l = (AAPAirwindowsPluginContext *) plugin->plugin_specific;
    delete l;
    delete plugin;
}

AndroidAudioPluginFactory aap_factory{
    aap_airwindows_plugin_new,
    aap_airwindows_plugin_delete
};

AndroidAudioPluginFactory *GetAndroidAudioPluginFactory() { return &aap_factory; }
