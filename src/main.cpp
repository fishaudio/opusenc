#include <iostream>
#include <opus/opus.h>
#include <opus/opusenc.h>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <memory>
#include <cstdio>

namespace py = pybind11;

class OpusBufferedEncoder
{
private:
    OggOpusEnc *encoder;
    OggOpusComments *comments;
    int channels;

public:
    OpusBufferedEncoder(int sample_rate, int channels, int bitrate = OPUS_AUTO, int signal_type = 0, int encoder_complexity = 10, int decision_delay = 0)
        : encoder(nullptr), comments(nullptr), channels(channels)
    {
        if (channels < 1 || channels > 8)
        {
            throw py::value_error("Invalid channels, must be in range [1, 8] inclusive.");
        }
        if ((bitrate < 500 or bitrate > 512000) && bitrate != OPUS_BITRATE_MAX && bitrate != OPUS_AUTO)
        {
            throw py::value_error("Invalid bitrate, must be at least 512 and at most 512k bits/s.");
        }
        if (sample_rate < 8000 or sample_rate > 48000)
        {
            throw py::value_error("Invalid sample_rate, must be at least 8k and at most 48k.");
        }
        if (encoder_complexity > 10 || encoder_complexity < 0)
        {
            throw py::value_error("Invalid encoder_complexity, must be in range [0, 10] inclusive. The higher, the better quality at the given bitrate, but uses more CPU.");
        }
        if (decision_delay < 0)
        {
            throw py::value_error("Invalid decision_delay, must be at least 0.");
        }

        int error;
        comments = ope_comments_create();
        encoder = ope_encoder_create_pull(comments, sample_rate, channels, 0, &error);
        if (error != OPE_OK)
        {
            throw py::value_error("Failed to create Opus encoder");
        }

        if (ope_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate)) != OPE_OK)
        {
            throw py::value_error("Could not set bitrate.");
        }

        opus_int32 opus_signal_type;
        switch (signal_type)
        {
        case 0:
            opus_signal_type = OPUS_AUTO;
            break;
        case 1:
            opus_signal_type = OPUS_SIGNAL_MUSIC;
            break;
        case 2:
            opus_signal_type = OPUS_SIGNAL_VOICE;
            break;
        default:
            throw py::value_error("Invalid signal type, must be 0 (auto), 1 (music) or 2 (voice).");
        }

        if (ope_encoder_ctl(encoder, OPUS_SET_SIGNAL(opus_signal_type)) != OPE_OK)
        {
            throw py::value_error("Could not set signal type.");
        }

        if (ope_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(encoder_complexity)) != OPE_OK)
        {
            throw py::value_error("Could not set encoder complexity.");
        }

        // Set decision delay
        if (ope_encoder_ctl(encoder, OPE_SET_DECISION_DELAY(decision_delay)) != OPE_OK)
        {
            throw py::value_error("Could not set decision delay.");
        }
    }

    py::bytes write(const py::array_t<int16_t> &buffer)
    {
        if (buffer.ndim() != 2 || buffer.shape(1) != channels)
        {
            throw py::value_error("Buffer must have shape [samples, channels] and match the number of channels specified in the constructor.");
        }

        const int16_t *data = buffer.data();
        int samples = buffer.shape(0);

        std::vector<unsigned char> encoded_data;

        if (ope_encoder_write(encoder, data, samples) != OPE_OK)
        {
            throw py::value_error("Encoding failed");
        }

        unsigned char *packet;
        opus_int32 len;
        while (ope_encoder_get_page(encoder, &packet, &len, 1) != 0)
        {
            encoded_data.insert(encoded_data.end(), packet, packet + len);
        }

        return py::bytes(reinterpret_cast<char *>(encoded_data.data()), encoded_data.size());
    }

    py::bytes flush()
    {
        ope_encoder_drain(encoder);
        opus_int32 len;
        unsigned char *packet;
        std::vector<unsigned char> encoded_data;
        while (ope_encoder_get_page(encoder, &packet, &len, 1) != 0)
        {
            encoded_data.insert(encoded_data.end(), packet, packet + len);
        }
        return py::bytes(reinterpret_cast<char *>(encoded_data.data()), encoded_data.size());
    }

    void close()
    {
        if (encoder)
        {
            ope_encoder_destroy(encoder);
            encoder = nullptr;
        }
        if (comments)
        {
            ope_comments_destroy(comments);
            comments = nullptr;
        }
    }

    ~OpusBufferedEncoder()
    {
        close();
    }
};

PYBIND11_MODULE(opusenc, m)
{
    py::class_<OpusBufferedEncoder>(m, "OpusBufferedEncoder")
        .def(py::init<int, int, int, int, int, int>(),
             py::arg("sample_rate"), py::arg("channels"),
             py::arg("bitrate") = OPUS_AUTO, py::arg("signal_type") = 0, py::arg("encoder_complexity") = 10, py::arg("decision_delay") = 0)
        .def("write", &OpusBufferedEncoder::write)
        .def("flush", &OpusBufferedEncoder::flush)
        .def("close", &OpusBufferedEncoder::close);
}
