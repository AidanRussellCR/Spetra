#pragma once

namespace spetra {

    class Timer {
    public:
        void start();
        void tick();

        double delta_seconds() const;
        double elapsed_seconds() const;

        void set_max_delta_seconds(double seconds);

    private:
        double m_delta_seconds = 0.0;
        double m_elapsed_seconds = 0.0;
        double m_max_delta_seconds = 0.05; // cap for large dt spikes
        unsigned long long m_last_counter = 0;
        bool m_started = false;
    };

} // namespace spetra
