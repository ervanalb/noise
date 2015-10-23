#include <stdlib.h>

#include "noise.h"
#include "core/util.h"
#include "samples/samples.h"

nz_obj_p synth_drum(size_t length) {
    nz_obj_p output = nz_obj_create(nz_sample_type);
    if (output == NULL) return NULL;

    size_t new_length = nz_vector_set_size(output, length);
    if (new_length != length) return NULL;

    double * sample = NZ_CAST(double *, output);
    double phi = 0.0;
    
    double x[4] = {0}; 
    double y[4] = {0}; 
    double w[4] = {0}; 

    for (size_t i = 0; i < length; i++, sample++) {
        memmove(&x[1], &x[0], sizeof(x) - sizeof(double));
        x[0] = (rand() / (double) (RAND_MAX / 2)) - 1.0;
        x[0] *= 0.1;

        double fundamental_freq = 440.;
        // (osc / second) / (frames / second) 
        double delta_phi = 2 * M_PI * fundamental_freq / nz_frame_rate ;
        phi += delta_phi;
        //double alpha = 1.0 -  exp(-delta_phi);

        /*
        w[0] = alpha * x[0] + (1.0 - alpha) * w[0];
        w[1] = alpha * w[0] + (1.0 - alpha) * w[1];
        w[2] = alpha * w[1] + (1.0 - alpha) * w[2];
        w[3] = alpha * w[2] + (1.0 - alpha) * w[3];
        */

        // Biquad filter
        double Fs = 1.0 / nz_frame_rate;
        double f0 = fundamental_freq;
        double gain = 2.0;
        double Q = 10;

        double A = gain;
        double w0 = 2 * M_PI * f0 / Fs;
        double alpha = sin(w0) / (2 * Q);

        double b0 = 1 + alpha * A;
        double b1 = -2 * cos(w0);
        double b2 = 1 - alpha * A;
        double a0 = 1 + alpha / A;
        double a1 = -2 * cos(w0);
        double a2 = 1 - alpha / A;

        y[0] = (b0/a0)*x[0] + (b1/a0)*x[1] + (b2/a0)*x[2] - (a1/a0)*y[1] - (a2/a0)*y[2];

        *sample = y[0];
        memmove(&y[1], &y[0], sizeof(y) - sizeof(double));
    }

    return output;
}
