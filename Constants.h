const char* VERSION = "V1.4";
const float FILTERCUTOFF[128] = {20, 23, 26, 29, 32, 36, 40, 46, 53, 60, 69, 78, 87, 98, 109, 120, 132, 145, 157, 171, 186, 200, 215, 231, 247, 264, 282, 300, 319, 338, 357, 378, 399, 421, 444, 467, 491, 516, 541, 567, 594, 621, 650, 680, 710, 741, 774, 806, 841, 876, 912, 949, 987, 1027, 1068, 1110, 1152, 1196, 1242, 1290, 1338, 1388, 1439, 1491, 1547, 1603, 1661, 1723, 1783, 1843, 1915, 1975, 2047, 2119, 2191, 2263, 2347, 2419, 2503, 2587, 2683, 2767, 2863, 2959, 3055, 3163, 3259, 3367, 3487, 3595, 3715, 3835, 3967, 4099, 4231, 4363, 4507, 4663, 4807, 4963, 5131, 5287, 5467, 5635, 5815, 6007, 6199, 6403, 6607, 6823, 7039, 7267, 7495, 7735, 7987, 8239, 8503, 8779, 9055, 9343, 9643, 9955, 10267, 10603, 10939, 11287, 11647, 12000};
const float FILTERFREQS[128] = {20, 23, 26, 29, 32, 36, 40, 46, 53, 60, 69, 78, 87, 98, 109, 120, 132, 145, 157, 171, 186, 200, 215, 231, 247, 264, 282, 300, 319, 338, 357, 378, 399, 421, 444, 467, 491, 516, 541, 567, 594, 621, 650, 680, 710, 741, 774, 806, 841, 876, 912, 949, 987, 1027, 1068, 1110, 1152, 1196, 1242, 1290, 1338, 1388, 1439, 1491, 1547, 1603, 1661, 1723, 1783, 1843, 1915, 1975, 2047, 2119, 2191, 2263, 2347, 2419, 2503, 2587, 2683, 2767, 2863, 2959, 3055, 3163, 3259, 3367, 3487, 3595, 3715, 3835, 3967, 4099, 4231, 4363, 4507, 4663, 4807, 4963, 5131, 5287, 5467, 5635, 5815, 6007, 6199, 6403, 6607, 6823, 7039, 7267, 7495, 7735, 7987, 8239, 8503, 8779, 9055, 9343, 9643, 9955, 10267, 10603, 10939, 11287, 11647, 12000};
const float POWER[128] = {0, 0.00009, 0.0003, 0.0006, 0.001, 0.0016, 0.0022, 0.003, 0.004, 0.005, 0.0062, 0.0075, 0.0089, 0.0105, 0.0122, 0.014, 0.016, 0.018, 0.02, 0.022, 0.025, 0.027, 0.03, 0.033, 0.036, 0.039, 0.042, 0.045, 0.049, 0.052, 0.056, 0.06, 0.063, 0.068, 0.072, 0.076, 0.08, 0.085, 0.09, 0.094, 0.099, 0.104, 0.109, 0.115, 0.12, 0.126, 0.131, 0.137, 0.143, 0.149, 0.155, 0.161, 0.168, 0.174, 0.181, 0.188, 0.194, 0.201, 0.209, 0.216, 0.223, 0.231, 0.238, 0.246, 0.254, 0.262, 0.27, 0.278, 0.287, 0.295, 0.304, 0.313, 0.321, 0.33, 0.34, 0.349, 0.358, 0.368, 0.377, 0.387, 0.397, 0.407, 0.417, 0.427, 0.437, 0.448, 0.459, 0.469, 0.48, 0.491, 0.502, 0.513, 0.525, 0.536, 0.548, 0.56, 0.571, 0.583, 0.595, 0.608, 0.62, 0.632, 0.645, 0.658, 0.671, 0.684, 0.697, 0.71, 0.723, 0.737, 0.75, 0.764, 0.778, 0.792, 0.806, 0.82, 0.834, 0.849, 0.863, 0.878, 0.893, 0.908, 0.923, 0.938, 0.953, 0.969, 0.984, 1};
const float GLIDE[128] = {12, 32, 54, 76, 98, 120, 142, 164, 186, 208, 230, 252, 274, 296, 318, 340, 362, 384, 406, 428, 450, 472, 494, 516, 538, 560, 582, 604, 626, 648, 670, 692, 714, 736, 758, 780, 802, 824, 846, 868, 890, 912, 934, 956, 978, 1000, 1022, 1044, 1066, 1088, 1110, 1132, 1154, 1176, 1198, 1220, 1242, 1264, 1286, 1308, 1324, 1342, 1360, 1378, 1396, 1414, 1432, 1450, 1468, 1486, 1510, 1528, 1546, 1564, 1582, 1600, 1618, 1636, 1654, 1672, 1690, 1708, 1726, 1744, 1762, 1780, 1798, 1816, 1834, 1852, 1870, 1888, 1896, 1914, 1932, 1950, 1968, 1986, 2004, 2022, 2040, 2058, 2076, 2094, 2112, 2130, 2148, 2166, 2184, 2202, 2220, 2238, 2256, 2274, 2292, 2310, 2328, 2346, 2364, 2382, 2400, 2420, 2440, 2460, 2480, 2500, 2500};
const float NOTEFREQS[128] = {8.176, 8.662, 9.177, 9.723, 10.301, 10.913, 11.562, 12.25, 12.978, 13.75, 14.568, 15.434, 16.352, 17.324, 18.354, 19.445, 20.602, 21.827, 23.125, 24.5, 25.957, 27.5, 29.135, 30.868, 32.703, 34.648, 36.708, 38.891, 41.203, 43.654, 46.249, 48.999, 51.913, 55, 58.27, 61.735, 65.406, 69.296, 73.416, 77.782, 82.407, 87.307, 92.499, 97.999, 103.826, 110, 116.541, 123.471, 130.813, 138.591, 146.832, 155.563, 164.814, 174.614, 184.997, 195.998, 207.652, 220, 233.082, 246.942, 261.626, 277.183, 293.665, 311.127, 329.628, 349.228, 369.994, 391.995, 415.305, 440, 466.164, 493.883, 523.251, 554.365, 587.33, 622.254, 659.255, 698.456, 739.989, 783.991, 830.609, 880, 932.328, 987.767, 1046.502, 1108.731, 1174.659, 1244.508, 1318.51, 1396.913, 1479.978, 1567.982, 1661.219, 1760, 1864.655, 1975.533, 2093.005, 2217.461, 2349.318, 2489.016, 2637.02, 2793.826, 2959.955, 3135.963, 3322.438, 3520, 3729.31, 3951.066, 4186.009, 4434.922, 4698.636, 4978.032, 5274.041, 5587.652, 5919.911, 6271.927, 6644.875, 7040, 7458.62, 7902.133, 8372.018, 8869.844, 9397.273, 9956.063, 10548.08, 11175.3, 11839.82, 12543.85};
const float ENVTIMES[128] = {1, 2, 4, 6, 9, 14, 20, 26, 33, 41, 49, 58, 67, 78, 89, 99, 111, 124, 136, 150, 164, 178, 192, 209, 224, 241, 258, 276, 295, 314, 333, 353, 374, 395, 418, 440, 464, 489, 513, 539, 565, 592, 621, 650, 680, 710, 742, 774, 808, 843, 878, 915, 952, 991, 1031, 1073, 1114, 1158, 1202, 1250, 1297, 1346, 1396, 1448, 1502, 1558, 1614, 1676, 1735, 1794, 1864, 1923, 1994, 2065, 2136, 2207, 2289, 2360, 2443, 2525, 2620, 2702, 2797, 2891, 2985, 3092, 3186, 3292, 3410, 3516, 3634, 3752, 3882, 4012, 4142, 4272, 4413, 4567, 4708, 4862, 5027, 5180, 5357, 5522, 5699, 5888, 6077, 6278, 6478, 6691, 6903, 7127, 7351, 7587, 7835, 8083, 8343, 8614, 8885, 9169, 9464, 9770, 10077, 10408, 10738, 11080, 11434, 11700};
//const float ENVTIMES10[128] = {1, 2, 4, 6, 9, 14, 19, 25, 31, 38, 46, 54, 65, 78, 89, 99, 111, 124, 136, 150, 164, 178, 192, 209, 224, 241, 258, 276, 295, 314, 333, 353, 374, 395, 418, 440, 464, 489, 513, 539, 565, 592, 621, 650, 680, 710, 742, 774, 808, 843, 878, 915, 952, 991, 1031, 1073, 1114, 1158, 1202, 1250, 1297, 1346, 1396, 1448, 1502, 1558, 1614, 1676, 1735, 1794, 1864, 1923, 1994, 2065, 2136, 2207, 2289, 2360, 2443, 2525, 2620, 2702, 2797, 2891, 2985, 3092, 3186, 3292, 3410, 3516, 3634, 3752, 3882, 4012, 4142, 4272, 4413, 4567, 4708, 4862, 5027, 5180, 5357, 5522, 5699, 5888, 6077, 6278, 6478, 6691, 6903, 7127, 7351, 7587, 7835, 8083, 6100, 6400, 6700, 7000, 7300, 7800, 8100, 8400, 8800, 9300, 9700, 10000};
const float LFOTEMPO[128] = {0.050, 0.050, 0.055, 0.055, 0.060, 0.064, 0.064, 0.069, 0.072, 0.077, 0.077, 0.081, 0.080, 0.087, 0.087, 0.092, 0.100, 0.100, 0.100, 0.104, 0.109, 0.115, 0.115, 0.122, 0.130, 0.130, 0.139, 0.149, 0.160, 0.160, 0.172, 0.185, 0.196, 0.20, 0.20, 0.21, 0.22, 0.24, 0.24, 0.26, 0.27, 0.29, 0.29, 0.31, 0.34, 0.37, 0.37, 0.39, 0.4, 0.40, 0.42, 0.45, 0.47, 0.47, 0.50, 0.53, 0.57, 0.57, 0.6, 0.62, 0.62, 0.65, 0.69, 0.74, 0.74, 0.80, 0.80, 0.86, 0.86, 0.94, 1.00, 1.00, 1.10, 1.12, 1.14, 1.14, 1.17, 1.20, 1.30, 1.40, 1.50, 1.50, 1.60, 1.60, 1.7, 1.8, 1.9, 2.0, 2.2, 2.2, 2.4, 2.6, 2.8, 3.00, 3.00, 3.1, 3.2, 3.2, 3.4, 3.80, 3.80, 4.10, 4.40, 4.70, 4.70, 5.00, 5.40, 5.80, 6.00, 6.00, 6.20, 6.40, 6.40, 6.80, 6.80, 7.30, 7.60, 8.00, 8.60, 9.30, 9.90, 10.60, 10.60, 11.50, 12.20, 12.80, 12.80, 12.80};
const String LFOTEMPOSTR[128] = {"1/32", "1/32", "1/32", "1/32", "1/32", "1/32", "1/32", "1/32", "3/64", "3/64", "3/64", "3/64", "3/64", "3/64", "3/64", "3/64", "1/16", "1/16", "1/16", "1/16", "1/16", "1/16", "1/16", "1/16", "3/32", "3/32", "3/32", "3/32", "3/32", "3/32", "3/32", "3/32", "1/8", "1/8", "1/8", "1/8", "1/8", "1/8", "1/8", "1/8", "3/16", "3/16", "3/16", "3/16", "3/16", "3/16", "3/16", "3/16", "1/4", "1/4", "1/4", "1/4", "1/4", "1/4", "1/4", "1/4", "3/8", "3/8", "3/8", "3/8", "3/8", "3/8", "3/8", "3/8", "1/2", "1/2", "1/2", "1/2", "1/2", "1/2", "1/2", "1/2", "3/4", "3/4", "3/4", "3/4", "3/4", "3/4", "3/4", "3/4", "1", "1", "1", "1", "1", "1", "1", "1", "3/2", "3/2", "3/2", "3/2", "3/2", "3/2", "3/2", "3/2", "2", "2", "2", "2", "2", "2", "2", "2", "3", "3", "3", "3", "3", "3", "3", "3", "4", "4", "4", "4", "4", "4", "4", "4", "6", "6", "6", "6", "6", "6", "6", "6"};
const float PITCH_DETUNE[128] = {-5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
const float PITCHBEND[128] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
const float PITCHBEND_REVERSE[128] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const float KEYTRACKINGAMT[128] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1, 1};
#define DIV8192 (1.0f / 8192.0f)//For pitchbend
#define DIV127 (1.0f / 127.0f)
#define DIV12 (1.0f / 12.0f)
const float ROTARY[128] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127};
const float LINEAR[128] = {0, 0.008, 0.016, 0.024, 0.031, 0.039, 0.047, 0.055, 0.063, 0.071, 0.079, 0.087, 0.094, 0.102, 0.11, 0.118, 0.126, 0.134, 0.142, 0.15, 0.157, 0.165, 0.173, 0.181, 0.189, 0.197, 0.205, 0.213, 0.22, 0.228, 0.236, 0.244, 0.252, 0.26, 0.268, 0.276, 0.283, 0.291, 0.299, 0.307, 0.315, 0.323, 0.331, 0.339, 0.346, 0.354, 0.362, 0.37, 0.378, 0.386, 0.394, 0.402, 0.409, 0.417, 0.425, 0.433, 0.441, 0.449, 0.457, 0.465, 0.472, 0.48, 0.488, 0.496, 0.504, 0.512, 0.52, 0.528, 0.535, 0.543, 0.551, 0.559, 0.567, 0.575, 0.583, 0.591, 0.598, 0.606, 0.614, 0.622, 0.63, 0.638, 0.646, 0.654, 0.661, 0.669, 0.677, 0.685, 0.693, 0.701, 0.709, 0.717, 0.724, 0.732, 0.74, 0.748, 0.756, 0.764, 0.772, 0.78, 0.787, 0.795, 0.803, 0.811, 0.819, 0.827, 0.835, 0.843, 0.85, 0.858, 0.866, 0.874, 0.882, 0.89, 0.898, 0.906, 0.913, 0.921, 0.929, 0.937, 0.945, 0.953, 0.961, 0.969, 0.976, 0.984, 0.992, 1.00};
const float LINEARCENTREZERO[128] = { -1, -0.98, -0.97, -0.95, -0.93, -0.92, -0.9, -0.88, -0.87, -0.85, -0.83, -0.82, -0.8, -0.78, -0.77, -0.75, -0.73, -0.72, -0.7, -0.68, -0.67, -0.65, -0.63, -0.62, -0.6, -0.58, -0.57, -0.55, -0.53, -0.52, -0.5, -0.48, -0.47, -0.45, -0.43, -0.42, -0.4, -0.38, -0.37, -0.35, -0.33, -0.32, -0.3, -0.28, -0.27, -0.25, -0.23, -0.22, -0.2, -0.18, -0.17, -0.15, -0.13, -0.12, -0.1, -0.08, -0.07, -0.05, -0.03, -0.02, -0.01, 0, 0, 0, 0, 0, 0, 0.01, 0.02, 0.03, 0.05, 0.07, 0.08, 0.1, 0.12, 0.13, 0.15, 0.17, 0.18, 0.2, 0.22, 0.23, 0.25, 0.27, 0.28, 0.3, 0.32, 0.33, 0.35, 0.37, 0.38, 0.4, 0.42, 0.43, 0.45, 0.47, 0.48, 0.5, 0.52, 0.53, 0.55, 0.57, 0.58, 0.6, 0.62, 0.63, 0.65, 0.67, 0.68, 0.7, 0.72, 0.73, 0.75, 0.77, 0.78, 0.8, 0.82, 0.83, 0.85, 0.87, 0.88, 0.9, 0.92, 0.93, 0.95, 0.97, 0.98, 1};
const float LINEAR_FILTERMIXER[128] = {0, 0.008, 0.016, 0.024, 0.031, 0.039, 0.047, 0.055, 0.063, 0.071, 0.079, 0.087, 0.094, 0.102, 0.11, 0.118, 0.126, 0.134, 0.142, 0.15, 0.157, 0.165, 0.173, 0.181, 0.189, 0.197, 0.205, 0.213, 0.22, 0.228, 0.236, 0.244, 0.252, 0.26, 0.268, 0.276, 0.283, 0.291, 0.299, 0.307, 0.315, 0.323, 0.331, 0.339, 0.346, 0.354, 0.362, 0.37, 0.378, 0.386, 0.394, 0.402, 0.409, 0.417, 0.425, 0.433, 0.441, 0.449, 0.457, 0.465, 0.472, 0.48, 0.488, 0.496, 0.504, 0.512, 0.52, 0.528, 0.535, 0.543, 0.551, 0.559, 0.567, 0.575, 0.583, 0.591, 0.598, 0.606, 0.614, 0.622, 0.63, 0.638, 0.646, 0.654, 0.661, 0.669, 0.677, 0.685, 0.693, 0.701, 0.709, 0.717, 0.724, 0.732, 0.74, 0.748, 0.756, 0.764, 0.772, 0.78, 0.787, 0.795, 0.803, 0.811, 0.819, 0.827, 0.835, 0.843, 0.85, 0.858, 0.866, 0.874, 0.882, 0.89, 0.898, 0.906, 0.913, 0.921, 0.929, 0.937, 0.945, 0.953, 0.961, 0.976, 0.988, 1.0, -99, -99};//{LP...HP,BP,BP}
const float NEWSTRING8[128] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 125, 123, 121, 119, 117, 115, 113, 111, 109, 107, 105, 103, 101, 99, 97, 95, 93, 91, 89, 87, 85, 83, 81, 79, 77, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 9 ,7, 5 ,3, 1, 0 };
const float NEWSTRING16[128] = {0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127};
const int LINEAR_INVERSE[128] = {127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
const int LINEAR_FILTERMIXERSTR[128] = {0, 1, 2, 2, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 13, 13, 14, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 23, 24, 24, 25, 26, 27, 28, 28, 29, 30, 31, 31, 32, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 46, 47, 48, 49, 50, 50, 51, 52, 53, 54, 54, 55, 56, 57, 57, 58, 59, 60, 61, 61, 62, 63, 64, 65, 65, 66, 67, 68, 69, 69, 70, 71, 72, 72, 73, 74, 75, 76, 76, 77, 78, 79, 80, 80, 81, 82, 83, 83, 84, 85, 86, 87, 87, 88, 89, 90, 91, 91, 92, 93, 94, 94, 95, 96, 97, 98, 99, 100, 100};
const int LINEAR_NORMAL[128] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127};
const int LINEAR_NEGATIVE[128] = {0, -1, -2, -2, -3, -4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 13, 13, 14, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 23, 24, 24, 25, 26, 27, 28, 28, 29, 30, 31, 31, 32, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 46, 47, 48, 49, 50, 50, 51, 52, 53, 54, 54, 55, 56, 57, 57, 58, 59, 60, 61, 61, 62, 63, 64, 65, 65, 66, 67, 68, 69, 69, 70, 71, 72, 72, 73, 74, 75, 76, 76, 77, 78, 79, 80, 80, 81, 82, 83, 83, 84, 85, 86, 87, 87, 88, 89, 90, 91, 91, 92, 93, 94, 94, 95, 96, 97, 98, 99, 100, 100};
#define PITCHLFOOCTAVERANGE 2.0f//2 Oct range
#define RE_READ -9
#define MAXDETUNE 0.04f //4%
#define LFOMAXRATE 40.0f//40Hz
#define FILTERMODMIXERMAX 1.0f
#define FILTEROCTAVERANGE 7.0f//Better low frequencies but less accurate cutoff
#define GLIDEFACTOR 5000.0f//Maximum glide time
#define  NO_OF_VOICES 6
#define NO_OF_PARAMS 49
const char* INITPATCHNAME = "Initial Patch";
#define HOLD_DURATION 1000
const uint32_t CLICK_DURATION = 250;
#define PATCHES_LIMIT 999
const String INITPATCH = "Solina,1.00,0.43,0.00,0,0,0.99,1.00,0.00,1.00,0.47,0.00,12,-12,5,5,0,0.83,0.70,0.16,0.00,0.00,1.10,282.00,0.00,0.70,0.10";
