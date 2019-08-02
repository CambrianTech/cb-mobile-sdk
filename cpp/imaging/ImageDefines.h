//
//  ImageDefines.h
//  Cambrian
//
//  Created by Joel Teply on 6/28/12.
//
//

#ifndef ImageDefines_h
#define ImageDefines_h

#define MIN3(x,y,z)  ((y) <= (z) ? \
((x) <= (y) ? (x) : (y)) \
: \
((x) <= (z) ? (x) : (z)))

#define MAX3(x,y,z)  ((y) >= (z) ? \
((x) >= (y) ? (x) : (y)) \
: \
((x) >= (z) ? (x) : (z)))

#define MIN_SCALAR(rgb) MIN3(rgb.val[0], rgb.val[1], rgb.val[2])
#define MAX_SCALAR(rgb) MAX3(rgb.val[0], rgb.val[1], rgb.val[2])
#define AVG_SCALAR(rgb) ((rgb.val[0] + rgb.val[1] + rgb.val[2]) / 3)

//vec3(0.2125, 0.7154, 0.0721)
#define INTENSITY(rgb) ((int) (0.2125 * rgb.val[0] + 0.7154 * rgb.val[1] + 0.0721 * rgb.val[2]))
#define INTENSITY_RGB(r,g,b) (int) (0.2125 * r + 0.7154 * g + 0.0721 * b)

#define DIF(a,b) (abs(a-b))

#define LIMIT(value, min, max) MAX(min, MIN(max, value))
#define LIMIT_255(value) LIMIT(value, 0, 255)
#define LIMIT_SCALAR(scalar) { \
scalar[0] = LIMIT_255(scalar.val[0]); \
scalar[1] = LIMIT_255(scalar.val[1]); \
scalar[2] = LIMIT_255(scalar.val[2]); \
}

#define MULTIPLY_255(numberA, numberB) LIMIT_255(((1 + numberA) * (1 + numberB) >> 8) - 1)

#endif
