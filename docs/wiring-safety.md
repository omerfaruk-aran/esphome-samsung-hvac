# Interface and wiring safety boundary

No physical interface, voltage, polarity, termination or model-specific wiring has been validated by this project. Do not connect an ESP32 directly to an unknown HVAC bus. HVAC boards can expose hazardous voltage or damage equipment; use qualified assistance and equipment/interface documentation for the identified setup.

F1/F2 and F3/F4 roles are model/controller-specific. A community report using one pair is not universal wiring guidance. The GPIO numbers and9600 8E1settings in software fixtures establish a build/validation contract, not permission to connect those pins to an HVAC terminal.

Before any later authorized pilot, record indoor/outdoor/controller model and firmware, bus/interface documentation, isolation and grounding arrangement, UART/RS485 adapter model, direction polarity and existing controller topology. Verify the chosen interface supports the actual line characteristics. Do not generalize resistor/termination or supply-voltage advice from unrelated devices. No component setting can establish electrical isolation.

Within one ESPHome setup, one physical bus must have one transport owner and serialized TX scheduler. Do not combine the legacy and new integration, raw UART actions, dummy receivers or independent product UART readers on that bus. Separate physical buses may use separate exclusive roots. GPIO ownership checks cannot identify an incorrectly wired external topology.

Production remains receive-only, but installing hardware, powering a bridge or changing firmware still requires a verified setup and an authorized procedure. The optional direction pin is a transport configuration field; its inactive level is not proof that an unidentified transceiver is electrically passive. Physical recovery, collision behavior and equipment response are NOT TESTED.
