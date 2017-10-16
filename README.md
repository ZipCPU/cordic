This repository is designed to hold a variety of demonstration sinewave
generators.  These sinewave generators will be discussed and used as examples on
[zipcpu.com](http://zipcpu.com).  If you watch carefully, you may find examples
here before they are posted, as I'm going to be doing my development here.

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

1. [Sine-wave table-based generator](http://zipcpu.com/dsp/2017/07/11/simplest-sinewave-generator.html)

1. [Quarter-Wave sine table lookup](http://zipcpu.com/dsp/2017/08/26/quarterwave.html)

1. [CORDIC generation of sines and cosines](http://zipcpu.com/dsp/2017/08/30/cordic.html)

1. [CORDIC generation of an arctan](http://zipcpu.com/dsp/2017/09/01/topolar.html)

1. A [CORDIC testbench](http://zipcpu.com/dsp/2017/10/02/cordic-tb.html) for
   the [sine/cosine generation capability](http://zipcpu.com/dsp/2017/08/30/cordic.html)

   - Depends upon a discussion of [statistical quantization effects](http://zipcpu.com/dsp/2017/09/27/quantization.html)

Another post on [the ZipCPU blog](http://zipcpu.com) discussed [how a CORDIC can be used](http://zipcpu.com/dsp/2017/09/16/pwm-demo.html) to evaluate an [improved(?) PWM signal](http://zipcpu.com/dsp/2017/09/04/pwm-reinvention.html).

## License

All of the source code in this repository is released under the
GPLv3.
