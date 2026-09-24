import os

import psycopg
from psycopg.types.json import Jsonb


def get_database_url():
    database_url = os.environ.get("DATABASE_URL")

    if not database_url:
        raise RuntimeError("DATABASE_URL environment variable is not set.")

    return database_url


def save_benchmark(payload):
    sql = """
        INSERT INTO benchmark_runs
        (
            device_path,
            configured_dpi,
            configured_polling_hz,

            sample_count,
            motion_sample_count,
            zero_motion_sample_count,

            average_event_rate_hz,
            estimated_polling_rate_hz,
            polling_rate_percent,

            mean_active_interval_ms,
            median_active_interval_ms,
            active_interval_stddev_ms,
            active_interval_variance_ms2,

            measured_dpi,
            peak_observed_ips,

            raw_input_gap_candidates,
            tracking_loss_candidates,
            spinout_candidates,
            negative_acceleration_candidates,

            missing_hid_sequence_supported,

            raw_payload
        )
        VALUES
        (
            %s, %s, %s,
            %s, %s, %s,
            %s, %s, %s,
            %s, %s, %s, %s,
            %s, %s,
            %s, %s, %s, %s,
            %s,
            %s
        )
        RETURNING id;
    """

    values = (
        payload["device_path"],
        payload["configured_dpi"],
        payload["configured_polling_hz"],

        payload["sample_count"],
        payload["motion_sample_count"],
        payload["zero_motion_sample_count"],

        payload["average_event_rate_hz"],
        payload["estimated_polling_rate_hz"],
        payload["polling_rate_percent"],

        payload["mean_active_interval_ms"],
        payload["median_active_interval_ms"],
        payload["active_interval_stddev_ms"],
        payload["active_interval_variance_ms2"],

        payload["measured_dpi"],
        payload["peak_observed_ips"],

        payload["raw_input_gap_candidates"],
        payload["tracking_loss_candidates"],
        payload["spinout_candidates"],
        payload["negative_acceleration_candidates"],

        payload["missing_hid_sequence_supported"],

        Jsonb(payload)
    )

    with psycopg.connect(get_database_url()) as connection:
        with connection.cursor() as cursor:
            cursor.execute(sql, values)

            row = cursor.fetchone()

            return row[0]