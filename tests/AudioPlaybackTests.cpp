// Windows-only playback verification. The game itself uses SFML audio exclusively.
#define NOMINMAX
#include "Frontend.hpp"
#include <algorithm>
#include <audioclient.h>
#include <audiopolicy.h>
#include <cmath>
#include <cstring>
#include <endpointvolume.h>
#include <filesystem>
#include <fstream>
#include <mmdeviceapi.h>
#include <windows.h>

// This MinGW SDK only forward-declares this standard Windows COM interface.
MIDL_INTERFACE("C02216F6-8C67-4B5B-9D00-D008E73E0064") IAudioMeterInformation : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetPeakValue(float *) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMeteringChannelCount(UINT *) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetChannelsPeakValues(UINT, float *) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryHardwareSupport(DWORD *) = 0;
};
__CRT_UUID_DECL(IAudioMeterInformation, 0xc02216f6, 0x8c67, 0x4b5b, 0x9d, 0x00, 0xd0, 0x08, 0xe7,
                0x3e, 0x00, 0x64)

class PlaybackProbe {
    IMMDeviceEnumerator *enumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioSessionManager2 *manager = nullptr;
    IAudioMeterInformation *meter = nullptr;
    IAudioClient *client = nullptr;
    IAudioCaptureClient *capture = nullptr;
    WAVEFORMATEX *format = nullptr;

  public:
    float lastLoopback = 0;
    ~PlaybackProbe() {
        if (client)
            client->Stop();
        if (capture)
            capture->Release();
        if (client)
            client->Release();
        if (format)
            CoTaskMemFree(format);
        if (meter)
            meter->Release();
        if (manager)
            manager->Release();
        if (device)
            device->Release();
        if (enumerator)
            enumerator->Release();
    }
    bool open(std::ofstream &report) {
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      __uuidof(IMMDeviceEnumerator), (void **)&enumerator);
        if (FAILED(hr))
            return false;
        if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device)))
            return false;
        if (FAILED(device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr,
                                    (void **)&manager)))
            return false;
        IAudioSessionEnumerator *sessions = nullptr;
        if (FAILED(manager->GetSessionEnumerator(&sessions)))
            return false;
        int count = 0;
        sessions->GetCount(&count);
        bool audible = false;
        for (int i = 0; i < count; ++i) {
            IAudioSessionControl *control = nullptr;
            IAudioSessionControl2 *info = nullptr;
            ISimpleAudioVolume *volume = nullptr;
            if (FAILED(sessions->GetSession(i, &control)))
                continue;
            if (SUCCEEDED(
                    control->QueryInterface(__uuidof(IAudioSessionControl2), (void **)&info))) {
                DWORD pid = 0;
                info->GetProcessId(&pid);
                if (pid == GetCurrentProcessId()) {
                    control->QueryInterface(__uuidof(IAudioMeterInformation), (void **)&meter);
                    if (SUCCEEDED(control->QueryInterface(__uuidof(ISimpleAudioVolume),
                                                          (void **)&volume))) {
                        float gain = 0;
                        BOOL muted = true;
                        volume->GetMasterVolume(&gain);
                        volume->GetMute(&muted);
                        report << "Windows app volume=" << gain << ", muted=" << muted << '\n';
                        audible = gain > 0 && !muted;
                        volume->Release();
                    }
                }
                info->Release();
            }
            control->Release();
        }
        sessions->Release();
        if (!meter || !audible)
            return false;
        if (FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&client)))
            return false;
        if (FAILED(client->GetMixFormat(&format)))
            return false;
        bool floating = format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT;
        if (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
            floating = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(format)->SubFormat.Data1 ==
                       WAVE_FORMAT_IEEE_FLOAT;
        if (!floating || format->wBitsPerSample != 32)
            return false;
        if (FAILED(client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
                                      10000000, 0, format, nullptr)))
            return false;
        if (FAILED(client->GetService(__uuidof(IAudioCaptureClient), (void **)&capture)))
            return false;
        report << "Output loopback: " << format->nSamplesPerSec << " Hz, " << format->nChannels
               << " channels\n";
        return SUCCEEDED(client->Start());
    }
    float measure(pocket::SoundSystem &sound, int milliseconds) {
        float peak = 0;
        lastLoopback = 0;
        for (int ms = 0; ms < milliseconds; ms += 10) {
            sound.update(.01f);
            sf::sleep(sf::milliseconds(10));
            float value = 0;
            if (meter && SUCCEEDED(meter->GetPeakValue(&value)))
                peak = std::max(peak, value);
            UINT32 size = 0;
            if (capture)
                capture->GetNextPacketSize(&size);
            while (size) {
                BYTE *data = nullptr;
                UINT32 frames = 0;
                DWORD flags = 0;
                if (FAILED(capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr)))
                    break;
                if (data && !(flags & AUDCLNT_BUFFERFLAGS_SILENT))
                    for (UINT32 i = 0; i < frames * format->nChannels; ++i) {
                        float sample;
                        std::memcpy(&sample, data + i * sizeof(float), sizeof(float));
                        lastLoopback = std::max(lastLoopback, std::abs(sample));
                    }
                capture->ReleaseBuffer(frames);
                capture->GetNextPacketSize(&size);
            }
        }
        return peak;
    }
};
int main() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    int failures = 0, checks = 0;
    {
        std::ofstream report("build/audio-playback-results.txt");
        auto expect = [&](bool passed, const std::string &label) {
            ++checks;
            report << (passed ? "PASS: " : "FAIL: ") << label << '\n';
            report.flush();
            if (!passed)
                ++failures;
        };
        pocket::SoundSystem sound;
        expect(sound.load(), "all effect assets load and menu stream starts");
        pocket::Settings settings;
        sound.volumes(settings);
        sound.update(.5f);
        sf::sleep(sf::milliseconds(300));
        PlaybackProbe probe;
        if (!probe.open(report)) {
            report << "Playback capture unavailable; no output claim can be made.\n";
            return 2;
        }
        for (int scene = 0; scene < 4; ++scene) {
            sound.setScene(scene);
            float peak = probe.measure(sound, 2200);
            expect(peak > .0001f && probe.lastLoopback > .0001f,
                   "music scene " + std::to_string(scene) +
                       " reaches app meter and output loopback; peak=" + std::to_string(peak));
        }
        settings.music = 0;
        sound.volumes(settings);
        for (int cue = 0; cue < static_cast<int>(pocket::Cue::Count); ++cue) {
            sound.pause(true);
            sound.pause(false);
            probe.measure(sound, 200); // Clear the preceding cue from the session meter.
            sound.play(cue);
            float peak = probe.measure(sound, 650);
            expect(peak > .0001f, "effect " + std::to_string(cue) +
                                      " reaches app output; peak=" + std::to_string(peak));
        }
        for (bool win : {true, false}) {
            sound.pause(true);
            sound.pause(false);
            probe.measure(sound, 200);
            sound.result(win);
            float peak = probe.measure(sound, 2000);
            expect(peak > .0001f,
                   win ? "victory sting produces output" : "defeat sting produces output");
        }
        settings.muted = true;
        sound.volumes(settings);
        probe.measure(sound, 300);
        sound.play(pocket::Cue::Power);
        expect(probe.measure(sound, 450) < .00001f, "mute silences actual app output");
        settings.muted = false;
        settings.music = 50;
        sound.volumes(settings);
        expect(probe.measure(sound, 600) > .0001f, "unmute restores output");
        sound.pause(true);
        probe.measure(sound, 300);
        expect(probe.measure(sound, 400) < .00001f, "sleep silences actual app output");
        sound.pause(false);
        expect(probe.measure(sound, 600) > .0001f, "wake restores output");
        sound.pause(true);
        // Deliberately missing one cue must leave music and other cues operational.
        const std::string root = "build/audio-missing-cue-" +
                                 std::to_string(GetCurrentProcessId()) + "-" +
                                 std::to_string(GetTickCount64());
        std::filesystem::create_directories(root + "/effects");
        std::filesystem::create_directories(root + "/music");
        for (int cue = 0; cue < static_cast<int>(pocket::Cue::Count) - 1; ++cue) {
            std::string name = (cue < 10 ? "0" : "") + std::to_string(cue) + ".ogg";
            std::filesystem::copy_file("assets/audio/effects/" + name, root + "/effects/" + name,
                                       std::filesystem::copy_options::overwrite_existing);
        }
        std::filesystem::copy_file("assets/audio/music/menu.ogg", root + "/music/menu.ogg",
                                   std::filesystem::copy_options::overwrite_existing);
        pocket::SoundSystem partial;
        expect(!partial.load(root), "missing asset reports incomplete audio");
        partial.volumes(settings);
        expect(probe.measure(partial, 900) > .0001f, "one missing cue does not disable music");
        settings.music = 0;
        partial.volumes(settings);
        probe.measure(partial, 300);
        partial.play(pocket::Cue::WrongAlt2);
        expect(probe.measure(partial, 650) > .0001f, "missing cue plays the fallback effect");
        partial.pause(true);
        report << checks - failures << " / " << checks << " audio playback checks passed\n";
    }
    CoUninitialize();
    return failures ? 1 : 0;
}
