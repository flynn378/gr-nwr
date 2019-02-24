/* -*- c++ -*- */
/* 
 * Copyright 2019 Andy Walls <awalls.cx18@gmail.com>.
 *
 * This file was automatically generated by gr_modtool from GNU Radio
 *
 * This file was automatically generated from a template incorporating
 * data input by Andy Walls and later hand edited by Andy Walls.
 * See http://www.gnu.org/licenses/gpl-faq.en.html#GPLOutput .
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include "fsk_cps_cfo_est_cc_impl.h"

#include <cmath>

namespace gr {
  namespace nwr {

    fsk_cps_cfo_est_cc::sptr
    fsk_cps_cfo_est_cc::make(float samp_rate,
                             float mod_index,
                             float baud_rate,
                             float tolerance,
                             int fft_size,
                             gr::fft::window::win_type window,
                             float beta,
                             const std::string &output_tag,
                             const std::string &sob_tag,
                             bool periodic,
                             float interval)
    {
      return gnuradio::get_initial_sptr
        (new fsk_cps_cfo_est_cc_impl(samp_rate,
                                     mod_index,
                                     baud_rate,
                                     tolerance,
                                     fft_size,
                                     window,
                                     beta,
                                     output_tag,
                                     sob_tag,
                                     periodic,
                                     interval));
    }

    /*
     * The private constructor
     */
    fsk_cps_cfo_est_cc_impl::fsk_cps_cfo_est_cc_impl(float samp_rate,
                                               float mod_index,
                                               float baud_rate,
                                               float tolerance,
                                               int fft_size,
                                               gr::fft::window::win_type window,
                                               float beta,
                                               const std::string &output_tag,
                                               const std::string &sob_tag,
                                               bool periodic,
                                               float interval)
      : gr::sync_block("fsk_cps_cfo_est_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_samp_rate(samp_rate),
        d_mod_index(mod_index),
        d_baud_rate(baud_rate),
        d_tolerance(tolerance),
        d_fft_size(fft_size),
        d_window(window),
        d_beta(beta),
        d_output_tag_key(pmt::intern(output_tag)),
        d_sob_tag_key(pmt::intern(sob_tag)),
        d_periodic(periodic),
        d_interval(interval),
        d_fft_window(NULL),
        d_x2(NULL),
        d_fft0(NULL),
        d_fft1(NULL),
        d_cps(NULL),
        d_mag2(NULL),
        d_n_bins(50),
        d_bin_mag_order(50, 0),
        d_target_bin_delta(0.0f),
        d_max_bin_error(0.0f),
        d_max_bin_err_int(0),
        d_tags(),
        d_src_id(pmt::intern(alias())),
        d_out_interval((uint64_t) lrint(interval * samp_rate)),
        d_out_next_off(0)
    {
        // Set our alignment for volk
        const int alignment_multiple =
                                      volk_get_alignment() / sizeof(gr_complex);
        set_alignment(std::max(1, alignment_multiple));

        // Try to reduce some of our overhead since we need d_fft_size + 1
        // samples to perform an estimate.  Calling this block repeatedly with
        // small input sample counts when the block wants to make an estimate
        // wastes CPU.
        set_output_multiple((d_fft_size + 1) * 2);

        // Initialize the periodic output tagging state
        if (d_interval <= 0.0f)
            d_periodic = false;

        if (d_out_interval < (uint64_t) d_fft_size + 1)
            d_out_interval = (uint64_t) d_fft_size + 1;

        // Build the window for the samples
        d_fft_window = (float *) volk_malloc(d_fft_size * sizeof(float),
                                             volk_get_alignment());
        if (d_fft_window == NULL)
            throw std::runtime_error("Failed to allocate memory for window");

        std::vector<float> tmp_win = gr::fft::window::build(d_window,
                                                            d_fft_size, d_beta);

        // Build an Fs/2 spectrum rotator into the window, to avoid having to
        // do explicit FFT shift operations to shift the result by Fs/2
        for (int i = 0; i < tmp_win.size(); i++)
            d_fft_window[i] = (i & 1) ? -tmp_win[i] : tmp_win[i];

        // Build the FFT objects and buffers
        d_fft0 = new gr::fft::fft_complex(d_fft_size, true, 1);
        d_fft1 = new gr::fft::fft_complex(d_fft_size, true, 1);

        // Allocate some intermediate results storage
        d_x2 = (gr_complex *) volk_malloc((d_fft_size + 1) * sizeof(gr_complex),
                                          volk_get_alignment());
        if (d_x2 == NULL)
            throw std::runtime_error("Failed to allocate memory for x^2");

        d_cps = (gr_complex *) volk_malloc(d_fft_size * sizeof(gr_complex),
                                           volk_get_alignment());
        if (d_cps == NULL)
            throw std::runtime_error("Failed to allocate memory for cross pwr");

        d_mag2 = (float *) volk_malloc(d_fft_size * sizeof(float),
                                       volk_get_alignment());
        if (d_mag2 == NULL)
            throw std::runtime_error("Failed to allocate memory for mag^2");

        // Compute some derived constants
        d_target_bin_delta = d_baud_rate * ((float) d_fft_size) / d_samp_rate
                             * (d_mod_index / 0.5f);
        d_max_bin_error = d_tolerance * ((float) d_fft_size) / d_samp_rate
                             * (d_mod_index / 0.5f);
        d_max_bin_err_int = (int) ceilf(d_max_bin_error);
        if (d_max_bin_err_int < 1)
            d_max_bin_err_int = 1;
#if 0
        std::cerr << "Constructor:" << " target_delta:" << d_target_bin_delta
                  << " max_bin_errf:" << d_max_bin_error
                  << " max_bin_erri:" << d_max_bin_err_int
                  << " n_bins:" << d_n_bins << " fft_size:" << d_fft_size
                  << " bin_mag_order.size():" << d_bin_mag_order.size()
                  << std::endl;
#endif
    }

    fsk_cps_cfo_est_cc_impl::~fsk_cps_cfo_est_cc_impl()
    {
        volk_free(d_mag2);
        d_mag2 = NULL;
        volk_free(d_cps);
        d_cps = NULL;
        volk_free(d_x2);
        d_x2 = NULL;
        delete d_fft1;
        d_fft1 = NULL;
        delete d_fft0;
        d_fft0 = NULL;
        volk_free(d_fft_window);
        d_fft_window = NULL;
    }

    double
    fsk_cps_cfo_est_cc_impl::compute_estimate(const gr_complex *in, bool &valid)
    {
        // Square the input signal to create spectral lines
        // separated in proportion to the baud rate
        volk_32fc_x2_multiply_32fc(d_x2, in, in, d_fft_size + 1);

        // Apply windows and copy into FFT input buffers
        // fft0 is 1 sample delayed compared to fft1.
        volk_32fc_32f_multiply_32fc(d_fft0->get_inbuf(),
                                    d_x2, d_fft_window, d_fft_size);
        volk_32fc_32f_multiply_32fc(d_fft1->get_inbuf(),
                                    d_x2 + 1, d_fft_window, d_fft_size);

        // Do the FFTs to compute the signal spectra
        d_fft0->execute();
        d_fft1->execute();

        // Compute the cross power spectrum
        volk_32fc_x2_multiply_conjugate_32fc(d_cps,
                                             d_fft1->get_outbuf(),
                                             d_fft0->get_outbuf(),
                                             d_fft_size);

        // and its magnitude^2 (since that's faster than magnitude)
        volk_32fc_magnitude_squared_32f(d_mag2, d_cps, d_fft_size);

        // Get the top N bins by mag^2 in descending order.
        // We no longer need d_mag2 after this, so we can destroy it in the
        // process.
        for (int i = 0; i < d_n_bins; i++) {
            volk_32f_index_max_32u(&d_bin_mag_order[i], d_mag2, d_fft_size);
            d_mag2[d_bin_mag_order[i]] = 0.0;
        }

        // Find the indices of the largest two bins the proper distance apart
        bool found = false;
        int delta = 0;
        float limit_ck = 0;
        // Loop through looking for properly spaced peaks, expanding the
        // allowed error limit, if a pass doesn't find an acceptable pair.
        int i, j;
        int limit;
        for (limit = 1; limit <= d_max_bin_err_int; limit++) {
            for (i = 0; i < (d_n_bins - 1); i++) {
                for (j = i + 1; j < d_n_bins; j++) {
                    // Taking care when subtracting uint32_t numbers
                    delta = d_bin_mag_order[i] > d_bin_mag_order[j]
                            ? d_bin_mag_order[i] - d_bin_mag_order[j]
                            : d_bin_mag_order[j] - d_bin_mag_order[i];
                    limit_ck = std::abs(d_target_bin_delta - (float) delta);
                    if (limit_ck <= (float) limit) {
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;
            }
            if (found)
                break;
        }

        // If we failed, return a non-answer
        if (!found) {
            valid = false;
            return 0.0;
        }

        valid = true;
        // The argument of any one bin of the cross power spectrum
        // is like an FM discriminator (i.e. quad_demod block) operating on
        // the frequencies in that bin of the (squared) signal spectrum.
        // We take the average of the two desired frequencies to find their
        // midpoint.
        // We then cut that in half, since we squared the original signal
        // which doubled the frequencies.
        // We then convert from normalized radian frequency to Hz.
        double w1, w2, ret;
        w1 = arg(d_cps[d_bin_mag_order[i]]);
        w2 = arg(d_cps[d_bin_mag_order[j]]);
        ret = ((w1 + w2) / 2.0) / 2.0 * (d_samp_rate / 2.0) / M_PI;
#if 0
        std::cerr << "cfo:" << ret << " w1:" << w1 << " w2:" << w2
                  << " cps[" << d_bin_mag_order[i] << "]:"
                  << d_cps[d_bin_mag_order[i]]
                  << " cps[" << d_bin_mag_order[j] << "]:"
                  << d_cps[d_bin_mag_order[j]]
                  << " i:" << i << " j:" << j
                  << " limit:" << limit << " limit_ck:" << limit_ck
                  << " delta:" << delta << " target_delta:" << d_target_bin_delta
                  << " max_bin_errf:" << d_max_bin_error
                  << " max_bin_erri:" << d_max_bin_err_int
                  << " n_bins:" << d_n_bins << " fft_size:" << d_fft_size
                  << " bin_mag_order.size():" << d_bin_mag_order.size()
                  << std::endl;
#endif
        return ret;
    }

    int
    fsk_cps_cfo_est_cc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        uint64_t nitems_rd = nitems_read(0);

        // Look for start of burst tags
        d_tags.clear();
        get_tags_in_range(d_tags, 0,
                          nitems_rd, nitems_rd + noutput_items, d_sob_tag_key);      
        if (d_periodic) {
            // Ensure the next periodic reporting interval we care about is not
            // in the past.
            while (d_out_next_off < nitems_rd)
                d_out_next_off += d_out_interval;
        }

        int idx;
        std::vector<tag_t>::iterator t;
        bool valid;
        double cfo;
        t = d_tags.begin();
        while (t != d_tags.end()) {
            // Is periodic CFO estimation enabled?
            if (d_periodic) {
                // Is this tag trumping the next periodic output?
                if (t->offset <= (d_out_next_off + (uint64_t) d_fft_size + 1)) {
                    // Reschedule the next periodic output to after this tag
                    d_out_next_off = t->offset + d_out_interval;

                    idx = static_cast<int>(t->offset - nitems_rd);
                    if (idx + d_fft_size + 1 > noutput_items) {
                        // We don't have enough items to do the estimate.
                        // Propagate input to output.
                        memcpy(out, in, sizeof(gr_complex) * idx);
                        // Return what we consumed and wait for more items.
                        return idx;
                    }
                    // Do the estimation
                    cfo = compute_estimate(&in[idx], valid);
                    if (valid) {
                        // Add the tag to the output
                        add_item_tag(0, t->offset, d_output_tag_key,
                                     pmt::from_double(cfo), d_src_id);
                    }
                    ++t;
                } else {
                    idx = static_cast<int>(d_out_next_off - nitems_rd);
                    if (idx + d_fft_size + 1 > noutput_items) {
                        // We don't have enough items to do the estimate.
                        // Propagate input to output.
                        if (idx > noutput_items)
                            idx = noutput_items;
                        memcpy(out, in, sizeof(gr_complex) * idx);
                        // Return what we consumed and wait for more items.
                        return idx;
                    }
                    // Do the estimation
                    cfo = compute_estimate(&in[idx], valid);
                    if (valid) {
                        // Add the tag to the output
                        add_item_tag(0, d_out_next_off, d_output_tag_key,
                                     pmt::from_double(cfo), d_src_id);
                    }
                    // Schedule the next periodic output
                    d_out_next_off += d_out_interval;
                }
            } else {
                idx = static_cast<int>(t->offset - nitems_rd);
                if (idx + d_fft_size + 1 > noutput_items) {
                    // We don't have enough items to do the estimate.
                    // Propagate input to output.
                    memcpy(out, in, sizeof(gr_complex) * idx);
                    // Return what we consumed and wait for more items.
                    return idx;
                }
                // Do the estimation
                cfo = compute_estimate(&in[idx], valid);
                if (valid) {
                    // Add the tag to the output
                    add_item_tag(0, t->offset, d_output_tag_key,
                                 pmt::from_double(cfo), d_src_id);
                }
                ++t;
            }
        }

        if (d_periodic) {
            while (1) {
                idx = static_cast<int>(d_out_next_off - nitems_rd);
                if (idx + d_fft_size + 1 > noutput_items) {
                    // We don't have enough items to do the estimate.
                    // Propagate input to output.
                    if (idx > noutput_items)
                        idx = noutput_items;
                    memcpy(out, in, sizeof(gr_complex) * idx);
                    // Return what we consumed and wait for more items.
                    return idx;
                }
                // Do the estimation
                cfo = compute_estimate(&in[idx], valid);
                if (valid) {
                    // Add the tag to the output
                    add_item_tag(0, d_out_next_off, d_output_tag_key,
                                 pmt::from_double(cfo), d_src_id);
                }
                // Schedule the next periodic output
                d_out_next_off += d_out_interval;
            }
        }

        // Propagate input to output.
        idx = noutput_items;
        memcpy(out, in, sizeof(gr_complex) * idx);
        return idx;
    }

  } /* namespace nwr */
} /* namespace gr */

