/* -*- c++ -*- */
/*
 * Copyright 2004,2011,2012 Free Software Foundation, Inc.
 * Copyright (C) 2016-2017  Andy Walls <awalls.cx18@gmail.com>
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_NWR_CLOCK_RECOVERY_MM_FF_H
#define	INCLUDED_NWR_CLOCK_RECOVERY_MM_FF_H

#include <nwr/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace nwr {
    
    /*!
     * \brief Mueller and Müller (M&M) based clock recovery block with float input, float output.
     * \ingroup synchronizers_blk
     *
     * \details
     * This implements the Mueller and Müller (M&M) discrete-time
     * error-tracking synchronizer.
     *
     * For this block to work properly, the input stream must meet the
     * following requirements:
     *
     * 1. the input pulses must have peaks and troughs (not flat), which
     * usually can be implemented by using a matched filter before this block.
     *
     * 2. the input pulse peaks and troughs should nominally be at +/-1.0
     * and centered about 0.0.
     *
     * See "Digital Communication Receivers: Synchronization, Channel
     * Estimation and Signal Processing" by Heinrich Meyr, Marc
     * Moeneclaey, & Stefan Fechtel.  ISBN 0-471-50275-8.
     */
    class NWR_API clock_recovery_mm_ff : virtual public block
    {
    public:
      // gr::nwr::clock_recovery_mm_ff::sptr
      typedef boost::shared_ptr<clock_recovery_mm_ff> sptr;

      /*!
       * Make a M&M clock recovery block.
       *
       * \param sps
       * User specified nominal clock period in samples per symbol.
       *
       * \param loop_bw
       * Approximate normailzed loop bandwidth of the symbol clock tracking
       * loop. It should nominally be close to 0, but greater than 0.  If
       * unsure, start with a number around 0.040, and experiment to find the
       * value that works best for your situation.
       *
       * \param damping_factor
       * Damping factor of the symbol clock tracking loop.
       * Damping < 1.0f is an under-damped loop.
       * Damping = 1.0f is a critically-damped loop.
       * Damping > 1.0f is an over-damped loop.
       * One should generally use an over-damped loop for clock recovery.
       *
       * \param max_deviation
       * Maximum absolute deviation of the average clock period estimate
       * from the user specified nominal clock period in samples per symbol.
       *
       * \param osps
       * The number of output samples per symbol (default=1).
       */
      static sptr make(float sps,
                       float loop_bw,
                       float damping_factor = 2.0f,
		       float max_deviation = 1.5f,
                       int osps = 1);
      
      virtual float loop_bandwidth() const = 0;
      virtual float damping_factor() const = 0;
      virtual float alpha() const = 0;
      virtual float beta() const = 0;

      virtual void set_loop_bandwidth (float fn_norm) = 0;
      virtual void set_damping_factor (float zeta) = 0;
      virtual void set_alpha (float alpha) = 0;
      virtual void set_beta (float beta) = 0;
    };

  } /* namespace nwr */
} /* namespace gr */

#endif /* INCLUDED_NWR_CLOCK_RECOVERY_MM_FF_H */
