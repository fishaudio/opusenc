import os

import numpy as np
import opusenc
import pytest


def test_write_read_opus():
    # Generate a simple sine wave
    sample_rate = 48000
    duration = 2  # seconds
    frequency = 440  # Hz
    t = np.linspace(0, duration, num=sample_rate * duration, endpoint=False)
    waveform = np.sin(2 * np.pi * frequency * t)

    # Convert to int16 and reshape to [time, channels]
    waveform_int16 = (waveform * 32767).astype(np.int16)
    waveform_tc = waveform_int16.reshape(-1, 1)  # Mono audio

    # Write to file
    test_file = "test_audio.opus"
    opusenc.write(test_file, waveform_tc, sample_rate)

    # Read from file
    read_waveform, read_sample_rate = opusenc.read(test_file)

    # Check if sample rate is correct
    assert read_sample_rate == sample_rate

    # Check if waveform shape is correct
    assert read_waveform.shape == waveform_tc.shape

    # Check if waveform content is similar (allowing for some compression artifacts)
    assert (read_waveform - waveform_tc).max() < 0.1 * 32767

    # Clean up
    os.remove(test_file)


def test_invalid_inputs():
    sample_rate = 48000
    waveform = np.zeros((1000, 1), dtype=np.int16)

    # Test invalid bitrate
    with pytest.raises(ValueError):
        opusenc.write("test.opus", waveform, sample_rate, bitrate=100)

    # Test invalid sample rate
    with pytest.raises(ValueError):
        opusenc.write("test.opus", waveform, 5000)

    # Test invalid signal type
    with pytest.raises(ValueError):
        opusenc.write("test.opus", waveform, sample_rate, signal_type=3)

    # Test invalid encoder complexity
    with pytest.raises(ValueError):
        opusenc.write("test.opus", waveform, sample_rate, encoder_complexity=11)


def test_read_nonexistent_file():
    with pytest.raises(ValueError):
        opusenc.read("nonexistent_file.opus")


if __name__ == "__main__":
    pytest.main([__file__])
