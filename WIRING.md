Relay wiring for ESP-12 (ESP8266)

Overview

- Use a safe GPIO pin to control a transistor which drives the relay coil. **GPIO4 (D2)** is recommended.

Parts & recommended values

- NPN transistor (eg. 2N2222 / BC337)
- Base resistor: **1 kΩ**
- Flyback diode: **1N400x** (1N4001, 1N4004, etc.) across the coil
- Relay module (single channel) or mechanical relay coil rated for **5V**
- External 5V power supply for the relay coil (recommended), or a relay module with onboard driver
- Common ground between ESP and relay supply

Quick wiring

1. GPIO4 (D2) → 1k resistor → transistor base
2. Transistor emitter → GND
3. Transistor collector → one side of relay coil
4. Other side of relay coil → +5V (VCC)
5. Flyback diode across coil (cathode to +5V, anode to transistor collector)
6. Connect ESP GND and relay power GND together

Safety & tips

- Do NOT drive the relay coil directly from the ESP pin (current too high).
- If switching mains AC, prefer a properly rated relay or SSR and follow safety rules (isolate, enclosures, fuses).
- Consider using a relay module that includes an optocoupler or driver transistor and provides convenient screw terminals.

Diagram

- See `data/relay_diagram.svg` for a small wiring diagram (added to the project).

Quick export to PNG

- If you don't have ImageMagick or librsvg installed, open `data/export_svg_to_png.html` in your browser (double-click the file) and click **Carregar e mostrar** → then **Exportar PNG**. The PNG will be downloaded as `relay_diagram.png`.

External 5V supply example

If you plan to power the relay coil from an external 5V supply and control it with GPIO4 (D2) on the ESP-12, follow these steps:

1. Wiring
   - GPIO4 (D2) → base resistor (1 kΩ typical) → transistor base (eg. 2N2222 / BC337)
   - Transistor emitter → ESP GND
   - Transistor collector → one side of relay coil
   - Other side of relay coil → +5V (external supply VCC)
   - Flyback diode (1N400x) across the coil (cathode to +5V, anode to transistor collector)
   - Connect the external 5V supply GND to the ESP GND (common ground)

2. Sizing the base resistor
   - For most small 5V relays (30–100 mA coil current), a base resistor of **~1 kΩ** works safely.
   - If you want to calculate precisely: choose Ib ≈ Ic / 10, then Rb ≈ (3.3V − 0.7V) / Ib.

3. Notes & warnings
   - Always use a transistor or a driver (eg. ULN2003) unless your relay module explicitly supports direct 3.3V logic input.
   - If using a relay **module** with an onboard transistor/optocoupler, check whether the input is active‑low and whether it is 3.3V‑compatible (some modules expect 5V input). Adjust your code or wiring accordingly.
   - Keep grounds common: failing to connect grounds between the ESP and the relay supply is the most common cause of control failures.

Demo sketch (added)

- See `examples/relay_test.ino` for a small Arduino/ESP8266 sketch that toggles GPIO4 and demonstrates both active‑LOW and active‑HIGH modules.

If you'd like, I can also add:

- A small schematic PNG exported from the SVG, or
- More example sketches (e.g., using a MOSFET or ULN2003 driver).
