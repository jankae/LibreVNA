#!/usr/bin/env python3

import argparse
import csv
import math
import time
from pathlib import Path

from libreVNA import libreVNA


def db_angle(value):
    magnitude = abs(value)
    db = 20.0 * math.log10(magnitude) if magnitude > 0 else -300.0
    angle = math.degrees(math.atan2(value.imag, value.real))
    return db, angle


def touchstone_text(trace, touchstone_format="ri", reference_impedance=50.0):
    normalized_format = touchstone_format.lower()
    if normalized_format not in ("ri", "db"):
        raise ValueError("touchstone_format must be either 'ri' or 'db'")

    option_format = normalized_format.upper()
    lines = [
        "! Touchstone S1P file generated via LibreVNA SCPI",
        f"# Hz S {option_format} R {reference_impedance:g}",
    ]

    for frequency, s11 in trace:
        if normalized_format == "ri":
            lines.append(f"{frequency:.0f} {s11.real:.12e} {s11.imag:.12e}")
        else:
            db, angle = db_angle(s11)
            lines.append(f"{frequency:.0f} {db:.6f} {angle:.6f}")

    return "\n".join(lines) + "\n"


def write_csv(trace, file_obj):
    writer = csv.writer(file_obj)
    writer.writerow(
        [
            "Frequency_Hz",
            "S11_real",
            "S11_imag",
            "S11_magnitude",
            "S11_dB",
            "S11_phase_deg",
        ]
    )
    for frequency, s11 in trace:
        db, angle = db_angle(s11)
        writer.writerow(
            [
                f"{frequency:.0f}",
                f"{s11.real:.12e}",
                f"{s11.imag:.12e}",
                f"{abs(s11):.12e}",
                f"{db:.12e}",
                f"{angle:.12e}",
            ]
        )


def wait_for_sweep(vna, timeout):
    deadline = time.time() + timeout
    while vna.query(":VNA:ACQ:FIN?") == "FALSE":
        if time.time() >= deadline:
            raise TimeoutError("Timed out waiting for the VNA sweep to finish")
        time.sleep(0.1)


def capture_s11(args):
    vna = libreVNA(args.host, args.port, timeout=args.timeout)
    print(vna.query("*IDN?"))

    vna.cmd(":DEV:CONN")
    device = vna.query(":DEV:CONN?")
    if device == "Not connected":
        raise RuntimeError("Not connected to any device")
    print("Connected to " + device)

    vna.cmd(":DEV:MODE VNA")
    if args.setup:
        vna.cmd(f":DEV:SETUP:LOAD {args.setup}")
    if args.calibration:
        vna.cmd(f":VNA:CAL:LOAD {args.calibration}")

    vna.cmd(":VNA:ACQ:STOP")
    vna.cmd(":VNA:SWEEP FREQUENCY")
    vna.cmd(f":VNA:ACQ:IFBW {args.ifbw}")
    vna.cmd(f":VNA:ACQ:AVG {args.averages}")
    vna.cmd(f":VNA:ACQ:POINTS {args.points}")
    vna.cmd(f":VNA:FREQ:START {args.start}")
    vna.cmd(f":VNA:FREQ:STOP {args.stop}")

    vna.cmd(":VNA:ACQ:SINGLE 1")
    vna.cmd(":VNA:ACQ:RUN")
    wait_for_sweep(vna, args.sweep_timeout)

    data = vna.query(":VNA:TRACE:DATA? S11", timeout=args.timeout)
    return vna.parse_VNA_trace_data(data)


def write_outputs(trace, output_base, touchstone_format):
    output_base = Path(output_base)
    csv_path = output_base.with_suffix(".csv")
    s1p_path = output_base.with_suffix(".s1p")

    with csv_path.open("w", newline="", encoding="utf-8") as csv_file:
        write_csv(trace, csv_file)

    s1p_path.write_text(touchstone_text(trace, touchstone_format), encoding="utf-8")
    return csv_path, s1p_path


def build_arg_parser():
    parser = argparse.ArgumentParser(
        description="Capture S11 from LibreVNA over SCPI and export CSV plus S1P files."
    )
    parser.add_argument("--host", default="localhost", help="LibreVNA-GUI SCPI host")
    parser.add_argument("--port", default=19542, type=int, help="LibreVNA-GUI SCPI port")
    parser.add_argument("--start", default=2.0e9, type=float, help="Start frequency in Hz")
    parser.add_argument("--stop", default=3.5e9, type=float, help="Stop frequency in Hz")
    parser.add_argument("--points", default=501, type=int, help="Number of sweep points")
    parser.add_argument("--ifbw", default=100, type=float, help="IF bandwidth in Hz")
    parser.add_argument("--averages", default=1, type=int, help="Number of averages")
    parser.add_argument("--setup", help="Optional LibreVNA setup file to load")
    parser.add_argument("--calibration", help="Optional LibreVNA calibration file to load")
    parser.add_argument("--output", default="S11_capture", help="Output path without extension")
    parser.add_argument(
        "--s1p-format",
        choices=("ri", "db"),
        default="ri",
        help="Touchstone output format: real/imaginary or dB/angle",
    )
    parser.add_argument("--timeout", default=30.0, type=float, help="Socket query timeout in seconds")
    parser.add_argument(
        "--sweep-timeout",
        default=60.0,
        type=float,
        help="Maximum time to wait for the sweep to finish",
    )
    return parser


def main():
    args = build_arg_parser().parse_args()
    trace = capture_s11(args)
    csv_path, s1p_path = write_outputs(trace, args.output, args.s1p_format)
    print(f"CSV saved: {csv_path.resolve()}")
    print(f"S1P saved: {s1p_path.resolve()}")


if __name__ == "__main__":
    main()
