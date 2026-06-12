# SCPI Programming Examples
This directory contains some basic examples, demonstrating the scripting capabilities of the LibreVNA using the SCPI interface. They are only intended as a starting point, for the complete list of available commands see the [SCPI Programming Guide](../ProgrammingGuide.pdf).

## How to run the examples
1. Connect the LibreVNA to your computer
2. Start the LibreVNA-GUI and make sure that the SCPI server is enabled (Window->Preferences->General). The examples use the default port (19542).
3. Use python3 to run an example

## Export S11 to CSV and S1P
`export_s11_s1p_csv.py` captures an S11 sweep and writes both a CSV file and a Touchstone S1P file:

```
python3 export_s11_s1p_csv.py --output S11_capture
```

By default, the S1P file is written in real/imaginary format. Use `--s1p-format db` to write dB/angle format instead.
