#include <cassert>
#include <iostream>
#include "ota_maintenance.h"

int main() {
    using Phase = OtaMaintenance::Phase;
    OtaMaintenance window;
    assert(!window.blocksBle());
    window.requestResume();
    assert(!window.blocksBle());

    window.requestRelease();
    assert(window.blocksBle());
    assert(window.phase() == Phase::Releasing);
    // A slow in-flight connection must not consume the flashing window.
    window.tick(900000);
    assert(window.phase() == Phase::Releasing);
    assert(window.remainingSeconds(900000) == 0);
    window.released(900000);
    assert(window.remainingSeconds(900000) == 180);
    assert(window.remainingSeconds(900001) == 180);
    assert(window.remainingSeconds(901000) == 179);
    window.tick(1079999);
    assert(window.phase() == Phase::Active);
    assert(window.remainingSeconds(1079999) == 1);
    window.tick(1080000);
    assert(window.phase() == Phase::Resuming);
    assert(window.blocksBle());
    window.resumed();
    assert(!window.blocksBle());

    // Manual resume and extension, including requests overtaking completion.
    window.requestRelease();
    window.released(1000);
    window.requestRelease();
    window.released(101000);
    assert(window.remainingSeconds(180000) == 101);
    window.requestResume();
    window.released(200000); // A stale completion cannot cancel resume.
    assert(window.phase() == Phase::Resuming);
    window.requestRelease();
    window.resumed(); // A stale completion cannot enable reconnect.
    assert(window.phase() == Phase::Releasing);
    window.released(300000);
    window.requestResume();
    window.resumed();
    assert(!window.blocksBle());

    // millis() wraps every ~49 days; the window still lasts exactly 180s.
    window.requestRelease();
    const uint32_t start = UINT32_MAX - 500;
    window.released(start);
    assert(window.remainingSeconds(start + uint32_t(1000)) == 179);
    window.tick(start + uint32_t(179999));
    assert(window.phase() == Phase::Active);
    window.tick(start + uint32_t(180000));
    assert(window.phase() == Phase::Resuming);

    // A reboot creates normal operation without changing the saved MAC.
    OtaMaintenance rebooted;
    assert(!rebooted.blocksBle());
    std::cout << "OTA maintenance lifecycle, races and rollover passed\n";
}
