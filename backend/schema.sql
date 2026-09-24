CREATE TABLE IF NOT EXISTS benchmark_runs
(
    id BIGSERIAL PRIMARY KEY,

    received_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    device_path TEXT NOT NULL,

    configured_dpi INTEGER NOT NULL,
    configured_polling_hz INTEGER NOT NULL,

    sample_count BIGINT NOT NULL,
    motion_sample_count BIGINT NOT NULL,
    zero_motion_sample_count BIGINT NOT NULL,

    average_event_rate_hz DOUBLE PRECISION NOT NULL,
    estimated_polling_rate_hz DOUBLE PRECISION NOT NULL,
    polling_rate_percent DOUBLE PRECISION NOT NULL,

    mean_active_interval_ms DOUBLE PRECISION NOT NULL,
    median_active_interval_ms DOUBLE PRECISION NOT NULL,
    active_interval_stddev_ms DOUBLE PRECISION NOT NULL,
    active_interval_variance_ms2 DOUBLE PRECISION NOT NULL,

    measured_dpi DOUBLE PRECISION,

    peak_observed_ips DOUBLE PRECISION NOT NULL,

    raw_input_gap_candidates INTEGER NOT NULL,
    tracking_loss_candidates INTEGER NOT NULL,
    spinout_candidates INTEGER NOT NULL,
    negative_acceleration_candidates INTEGER NOT NULL,

    missing_hid_sequence_supported BOOLEAN NOT NULL,

    raw_payload JSONB NOT NULL
);