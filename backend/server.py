import json
import socketserver

from database import save_benchmark


# WORKLOAD:
# Run benchmark for 10 seconds
# Send ONE summary
# Database insert
# Thats why one thread

HOST = "127.0.0.1"
PORT = 9000

MAX_MESSAGE_BYTES = 64 * 1024


REQUIRED_FIELDS = {
    "device_path",
    "configured_dpi",
    "configured_polling_hz",

    "sample_count",
    "motion_sample_count",
    "zero_motion_sample_count",

    "average_event_rate_hz",
    "estimated_polling_rate_hz",
    "polling_rate_percent",

    "mean_active_interval_ms",
    "median_active_interval_ms",
    "active_interval_stddev_ms",
    "active_interval_variance_ms2",

    "measured_dpi",
    "peak_observed_ips",

    "raw_input_gap_candidates",
    "tracking_loss_candidates",
    "spinout_candidates",
    "negative_acceleration_candidates",

    "missing_hid_sequence_supported"
}


def validate_payload(payload):
    if not isinstance(payload, dict):
        raise ValueError("Payload must be a JSON object.")

    missing_fields = REQUIRED_FIELDS - payload.keys()

    if missing_fields:
        missing = ", ".join(sorted(missing_fields))
        raise ValueError(f"Missing fields: {missing}")

    if payload["configured_dpi"] <= 0:
        raise ValueError("configured_dpi must be positive.")

    if payload["configured_polling_hz"] <= 0:
        raise ValueError("configured_polling_hz must be positive.")

    if payload["sample_count"] < 0:
        raise ValueError("sample_count cannot be negative.")


class BenchmarkRequestHandler(socketserver.StreamRequestHandler):
    def handle(self):
        line = self.rfile.readline(MAX_MESSAGE_BYTES + 1)

        if not line:
            return

        if len(line) > MAX_MESSAGE_BYTES:
            print("Rejected message: too large.")
            return

        try:
            payload = json.loads(line.decode("utf-8"))

            validate_payload(payload)

            benchmark_id = save_benchmark(payload)

            print(
                f"Saved benchmark {benchmark_id}: "
                f"{payload['configured_polling_hz']} Hz, "
                f"{payload['sample_count']} samples"
            )

        except json.JSONDecodeError as error:
            print(f"Invalid JSON: {error}")

        except (ValueError, KeyError, TypeError) as error:
            print(f"Invalid benchmark: {error}")

        except Exception as error:
            print(f"Backend error: {error}")


class BenchmarkServer(socketserver.TCPServer):
    allow_reuse_address = True


def main():
    print(f"Benchmark backend listening on {HOST}:{PORT}")

    with BenchmarkServer(
        (HOST, PORT),
        BenchmarkRequestHandler
    ) as server:
        server.serve_forever()


if __name__ == "__main__":
    main()