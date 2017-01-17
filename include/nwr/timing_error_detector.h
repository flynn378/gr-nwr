/* -*- c++ -*- */
/*
 * Copyright (C) 2017  Andy Walls <awalls.cx18@gmail.com>
 */

#ifndef INCLUDED_NWR_TIMING_ERROR_DETECTOR_H
#define INCLUDED_NWR_TIMING_ERROR_DETECTOR_H

#include <nwr/api.h>

#include <gnuradio/gr_complex.h>
#include <gnuradio/digital/constellation.h>
#include <deque>

namespace gr {
  namespace nwr {

    class timing_error_detector;

    class NWR_API timing_error_detector
    {
      public:

        // Timing Error Detector types.
        // A decision directed algorithm requires a constellation.
        enum ted_type {
            TED_NONE                         = -1,
            TED_MUELLER_AND_MULLER           = 0, // Decision directed
            TED_MOD_MUELLER_AND_MULLER       = 1, // Decision directed
            TED_ZERO_CROSSING                = 2, // Decision directed     
            TED_GARDNER                      = 4,
            TED_EARLY_LATE                   = 5,
            TED_DANDREA_AND_MENGALLI_GEN_MSK = 6, // Complex only
        };

        static timing_error_detector *make(
                                    enum ted_type type,
                                    digital::constellation_sptr constellation =
                                                 digital::constellation_sptr());

        virtual ~timing_error_detector() {};

        virtual int inputs_per_symbol() { return d_inputs_per_symbol; }

        virtual void input(const gr_complex &x) = 0;
        virtual void input(float x) { input(gr_complex(x, 0.0f)); }
        virtual float error() { return d_error; }

        virtual void revert() = 0;
        virtual void sync_reset() = 0;

      private:
        enum ted_type d_type;

      protected:
        timing_error_detector(enum ted_type type,
                              digital::constellation_sptr constellation =
                                                 digital::constellation_sptr());

        void advance_input_clock() {
            d_input_clock = (d_input_clock + 1) % d_inputs_per_symbol;
        }
        void revert_input_clock();
        void sync_reset_input_clock() {
            d_input_clock = d_inputs_per_symbol - 1;
        }

        gr_complex slice(const gr_complex &x);

        digital::constellation_sptr d_constellation;
        float d_error;
        float d_prev_error;
        int d_inputs_per_symbol;
        int d_input_clock;
    };

    class NWR_API ted_mueller_and_muller : public timing_error_detector
    {
      public:
        ted_mueller_and_muller(digital::constellation_sptr constellation);
        ~ted_mueller_and_muller() {};

        void input(const gr_complex &x);
        void input(float x);
        void revert();
        void sync_reset();

      private:
        std::deque<gr_complex> d_input;
        std::deque<gr_complex> d_decision;
    };

    class NWR_API ted_mod_mueller_and_muller : public timing_error_detector
    {
      public:
        ted_mod_mueller_and_muller(digital::constellation_sptr constellation);
        ~ted_mod_mueller_and_muller() {};

        void input(const gr_complex &x);
        void input(float x);
        void revert();
        void sync_reset();

      private:
        std::deque<gr_complex> d_input;
        std::deque<gr_complex> d_decision;
    };

  } /* namespace nwr */
} /* namespace gr */

#endif /* INCLUDED_NWR_TIMING_ERROR_DETECTOR_H */
