# Catzidze (Course-work-2-term) 

An old course project that is represented by a software tool for searching for a piece of music among the local library, including analyzing matching fragments in songs. An analogue of the Shazam application.

## Description
The project, based on the Cooley-Tukey algorithm, is an efficient implementation of the Fast Fourier Transform (FFT) algorithm using base two. This algorithm, also known as the "butterfly", demonstrates the unique ability to split an N-point transformation into two (N/2)-point transformations, which, in turn, continue to split into smaller transformations until the length of the transformation becomes one. After that, the reverse process begins, in which the lengths of the transformation results are doubled using "butterflies". Within the framework of this project, complex multiplications and additions are actively used.

The input data, represented as an array of complex numbers, can be divided into two parts: one with elements with even numbers and the other with elements with odd numbers. Using a recursive call to the algorithm, the simplest values are obtained, which are then glued together to form the final result. The result is an array where the first half is equal to the complex sum of the even half of the input data and the product of the turning factor ω (equal to ω = exp(-2πi/N)) to the corresponding element of the odd-half array. The second half of the resulting array has a similar formula, but the difference is used instead of the sum.

This project demonstrates the power and efficiency of the Cooley-Tukey algorithm for performing fast Fourier transform, making it an ideal choice for processing large amounts of data in digital signal processing and other applications requiring rapid frequency response analysis.

<p align="center">
  <img height="200" src="https://github.com/dumonten/Catzidze/assets/92388475/1e4b4783-50f6-4506-8e03-046d44b79b88" alt="main">
</p>

****

<p align="center">
  <img height="200" src="https://github.com/dumonten/Catzidze/assets/92388475/1c1b99d3-8e2e-4708-824d-757b8a04abd3" alt="list-search">
</p>

