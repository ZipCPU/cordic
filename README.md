This repository is designed to hold a variety of demonstration sinewave
generators.  These sinewave generators have been discussed and used as examples
as part of the [ZipCPU Blog on zipcpu.com](http://zipcpu.com).  If you watch
carefully, you may find examples here before they are posted, as I'm going to
be doing my development here.

The basic approach in this project is that of a software core generator,
found in the [sw](sw/) directory.  This generator can be used to make
logic in the [rtl](rtl/) directory.

The reference that I have used to build the CORDIC algorithms within this
repository comes from a [Cordic
Survey](http://www.andraka.com/files/crdcsrvy.pdf), by Ray Andraka.

## Blog posts

There have been several blog posts based upon the code within this repository.
These include:

1. [No PI for you!](http://zipcpu.com/dsp/2017/06/15/no-pi-for-you.html), a
   discussion of the ideal units of phase within an FPGA.

1. How to build the simplest sine-wave in Verilog, based upon a [table-based generator](http://zipcpu.com/dsp/2017/07/11/simplest-sinewave-generator.html)

1. How to do the same thing, but with only a [Quarter-Wave sine table](http://zipcpu.com/dsp/2017/08/26/quarterwave.html).

1. How to use a [CORDIC to generate both sines and cosines](http://zipcpu.com/dsp/2017/08/30/cordic.html).  Ultimately this is about how to convert from polar to rectangular coordinates as well.

1. How to use a [CORDIC to calculate an arctan](http://zipcpu.com/dsp/2017/09/01/topolar.html).  This is also known as the rectangular to polar conversion mode of the CORDIC.

1. A [CORDIC testbench](http://zipcpu.com/dsp/2017/10/02/cordic-tb.html) for
   the [sine/cosine generation capability](http://zipcpu.com/dsp/2017/08/30/cordic.html)

   - This [test bench](bench/cpp/cordic_tb.cpp) depends upon the discussion of [statistical quantization effects](http://zipcpu.com/dsp/2017/09/27/quantization.html) that will hinder the performance of the CORDIC.

Another post on [the ZipCPU blog](http://zipcpu.com) discussed [how a CORDIC can be used](http://zipcpu.com/dsp/2017/09/16/pwm-demo.html) to evaluate an [improved(?) PWM signal](http://zipcpu.com/dsp/2017/09/04/pwm-reinvention.html).

If time permits, I have another sine wave generator that uses fewer
logic resources than the CORDIC above, but that requires two multiplies and
three RAM's--yet doesn't suffer from the phase truncation effects of the CORDIC.
I look forward to having the opportunity to post this in the future.

## License

All of the source code in this repository is released under the
[GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html).  If these conditions
are not sufficient for your needs, other licenses terms may be purchased.
