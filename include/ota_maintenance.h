#pragma once

#include <stdint.h>

// Access under the gateway state mutex. No state is written to flash.
class OtaMaintenance {
public:
    enum class Phase { Normal, Releasing, Active, Resuming };
    static constexpr uint32_t durationMs = 180000;

    void requestRelease() { phase_ = Phase::Releasing; }
    void requestResume() {
        if (blocksBle()) phase_ = Phase::Resuming;
    }
    void released(uint32_t now) {
        if (phase_ != Phase::Releasing) return;
        startedAt_ = now;
        phase_ = Phase::Active;
    }
    void resumed() {
        if (phase_ == Phase::Resuming) phase_ = Phase::Normal;
    }
    void tick(uint32_t now) {
        if (phase_ == Phase::Active && uint32_t(now - startedAt_) >= durationMs) {
            phase_ = Phase::Resuming;
        }
    }
    bool blocksBle() const { return phase_ != Phase::Normal; }
    Phase phase() const { return phase_; }
    uint32_t remainingSeconds(uint32_t now) const {
        if (phase_ != Phase::Active) return 0;
        const uint32_t elapsed = now - startedAt_;
        return elapsed >= durationMs ? 0 : (durationMs - elapsed + 999) / 1000;
    }
    const char* name() const {
        switch (phase_) {
            case Phase::Releasing: return "releasing";
            case Phase::Active: return "active";
            case Phase::Resuming: return "resuming";
            default: return "normal";
        }
    }

private:
    Phase phase_ = Phase::Normal;
    uint32_t startedAt_ = 0;
};
