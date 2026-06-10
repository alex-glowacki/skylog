"""Basic smoke tests for the skylog package."""

import skylog


def test_version() -> None:
    assert skylog.__version__ == "0.1.0"