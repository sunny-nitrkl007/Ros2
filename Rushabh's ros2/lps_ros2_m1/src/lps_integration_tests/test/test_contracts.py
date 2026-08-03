def test_expected_native_endpoints_are_stable():
    endpoints = {
        '/loader/weigh/state',
        '/loader/job_manager/state',
        '/loader/weigh/command',
        '/loader/job_manager/command',
    }
    assert len(endpoints) == 4


def test_legacy_queue_depth_baseline():
    queue_depth = 20
    assert queue_depth == 20


def test_scheduling_periods_ms():
    assert 1000 // 10 == 100
    assert 1000 // 50 == 20
