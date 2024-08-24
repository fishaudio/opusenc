import librosa
import numpy as np
import opusenc

sample_rate = 48000
channels = 1
audio, sr = librosa.load("tests/test.wav", sr=sample_rate, mono=True)

# Convert to numpy array (16-bit integers, little-endian)
audio_array = (audio * 32767).astype(np.int16)
audio_array = audio_array.reshape(-1, channels)

# Create Opus encoder
encoder = opusenc.OpusBufferedEncoder(sample_rate, channels)

# Encode the audio
# encoded_data = encoder.write(audio_array)
encoded_data = b""
# Write in chunks
for idx in range(0, len(audio_array), 1024):
    encoded_data += encoder.write(audio_array[idx : idx + 1024])
encoded_data += encoder.flush()

# Write the encoded data to a file
with open("output.opus", "wb") as f:
    f.write(encoded_data)
