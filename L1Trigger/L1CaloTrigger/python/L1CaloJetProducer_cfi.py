import FWCore.ParameterSet.Config as cms

L1CaloJetProducer = cms.EDProducer("L1CaloJetProducer",
    debug = cms.bool(False),
    HcalTpEtMin = cms.double(0.5),
    EcalTpEtMin = cms.double(0.5),
    EtMinForSeedHit = cms.double(2.5),
    l1CaloTowers = cms.InputTag("L1EGammaClusterEmuProducer","L1CaloTowerCollection","L1AlgoTest"),
    L1CrystalClustersInputTag = cms.InputTag("L1EGammaClusterEmuProducer", "L1EGXtalClusterEmulator", "L1AlgoTest"),

	emFractionBins = cms.vdouble([ 0.00,0.10,0.15,0.19,0.23,0.27,0.32,0.36,0.42,0.52,1.05]),
    absEtaBins = cms.vdouble([ 0.00,0.30,0.70,2.00]),
    jetPtBins = cms.vdouble([ 0.0,5.0,7.5,10.0,12.5,15.0,17.5,20.0,22.5,25.0,27.5,30.0,35.0,40.0,45.0,50.0,55.0,60.0,65.0,70.0,75.0,80.0,85.0,90.0,95.0,100.0,110.0,120.0,130.0,140.0,150.0,160.0,170.0,180.0,190.0,200.0,225.0,250.0,275.0,300.0,325.0,400.0,500.0]),
    jetCalibrations = cms.vdouble([
		4.789, 2.628, 2.038, 1.394, 1.211, 0.898, 0.775, 0.743, 0.626, 0.608, 0.542, 0.474, 0.439, 0.427, 0.423, 0.424, 0.412, 0.435, 0.440, 0.441, 0.438, 0.493, 0.516, 0.497, 0.485, 0.534, 0.555, 0.603, 0.572, 0.605, 0.610, 0.637, 0.638, 0.664, 0.692, 0.717, 0.780, 0.852, 0.839, 0.927, 1.002, 1.241,
		4.336, 2.391, 1.686, 1.368, 1.193, 0.957, 0.802, 0.709, 0.654, 0.572, 0.529, 0.467, 0.422, 0.389, 0.365, 0.344, 0.330, 0.345, 0.350, 0.346, 0.369, 0.370, 0.376, 0.408, 0.424, 0.449, 0.464, 0.470, 0.495, 0.482, 0.527, 0.525, 0.559, 0.571, 0.576, 0.593, 0.624, 0.663, 0.735, 0.789, 0.844, 1.023,
		4.516, 2.332, 1.670, 1.299, 1.076, 0.890, 0.760, 0.689, 0.596, 0.556, 0.511, 0.451, 0.397, 0.355, 0.321, 0.306, 0.284, 0.265, 0.265, 0.251, 0.257, 0.255, 0.262, 0.267, 0.284, 0.288, 0.314, 0.330, 0.350, 0.376, 0.386, 0.404, 0.417, 0.435, 0.428, 0.458, 0.483, 0.517, 0.546, 0.567, 0.618, 0.734,
		2.950, 2.626, 1.792, 1.377, 1.130, 0.936, 0.853, 0.742, 0.670, 0.623, 0.511, 0.510, 0.446, 0.422, 0.382, 0.384, 0.422, 0.407, 0.440, 0.470, 0.459, 0.512, 0.525, 0.480, 0.540, 0.553, 0.578, 0.610, 0.611, 0.611, 0.649, 0.647, 0.674, 0.700, 0.689, 0.726, 0.756, 0.783, 0.839, 0.847, 0.977, 1.076,
		2.910, 2.746, 1.725, 1.337, 1.034, 0.908, 0.764, 0.666, 0.641, 0.560, 0.519, 0.461, 0.417, 0.379, 0.335, 0.324, 0.334, 0.326, 0.349, 0.354, 0.366, 0.406, 0.394, 0.428, 0.458, 0.480, 0.497, 0.511, 0.525, 0.549, 0.569, 0.569, 0.591, 0.581, 0.591, 0.626, 0.648, 0.674, 0.700, 0.714, 0.825, 0.878,
		4.867, 2.544, 1.753, 1.333, 1.044, 0.929, 0.774, 0.666, 0.591, 0.524, 0.487, 0.433, 0.371, 0.330, 0.302, 0.289, 0.265, 0.262, 0.265, 0.276, 0.273, 0.297, 0.301, 0.313, 0.330, 0.364, 0.384, 0.397, 0.408, 0.430, 0.443, 0.451, 0.463, 0.467, 0.487, 0.493, 0.505, 0.517, 0.538, 0.554, 0.597, 0.666,
		4.170, 2.380, 1.712, 1.486, 1.108, 0.952, 0.808, 0.710, 0.656, 0.559, 0.505, 0.478, 0.429, 0.396, 0.382, 0.405, 0.428, 0.428, 0.436, 0.491, 0.503, 0.515, 0.578, 0.572, 0.569, 0.584, 0.591, 0.629, 0.632, 0.650, 0.674, 0.697, 0.675, 0.718, 0.715, 0.732, 0.780, 0.791, 0.841, 0.866, 0.917, 1.079,
		3.861, 2.314, 2.065, 1.357, 1.082, 0.906, 0.808, 0.674, 0.605, 0.512, 0.479, 0.445, 0.384, 0.353, 0.336, 0.333, 0.335, 0.354, 0.358, 0.409, 0.421, 0.459, 0.453, 0.487, 0.498, 0.513, 0.523, 0.542, 0.561, 0.572, 0.578, 0.601, 0.598, 0.619, 0.619, 0.649, 0.658, 0.684, 0.693, 0.714, 0.776, 0.860,
		4.479, 2.541, 1.788, 1.376, 1.063, 0.911, 0.779, 0.691, 0.579, 0.532, 0.469, 0.412, 0.365, 0.317, 0.290, 0.284, 0.284, 0.289, 0.296, 0.308, 0.338, 0.339, 0.372, 0.377, 0.410, 0.402, 0.429, 0.458, 0.448, 0.466, 0.462, 0.480, 0.492, 0.496, 0.494, 0.517, 0.515, 0.534, 0.542, 0.562, 0.599, 0.655,
		4.807, 2.462, 1.854, 1.442, 1.172, 0.917, 0.770, 0.706, 0.602, 0.524, 0.496, 0.439, 0.422, 0.384, 0.394, 0.432, 0.448, 0.493, 0.525, 0.525, 0.564, 0.584, 0.567, 0.586, 0.599, 0.619, 0.668, 0.658, 0.702, 0.678, 0.697, 0.686, 0.723, 0.711, 0.709, 0.738, 0.764, 0.775, 0.801, 0.820, 0.878, 0.981,
		4.667, 2.545, 1.865, 1.403, 1.127, 0.957, 0.739, 0.675, 0.584, 0.528, 0.481, 0.432, 0.378, 0.340, 0.342, 0.363, 0.383, 0.399, 0.439, 0.455, 0.471, 0.478, 0.495, 0.526, 0.528, 0.557, 0.558, 0.576, 0.588, 0.601, 0.620, 0.624, 0.622, 0.641, 0.625, 0.647, 0.675, 0.697, 0.704, 0.724, 0.760, 0.834,
		4.915, 2.584, 1.744, 1.392, 1.159, 0.880, 0.802, 0.664, 0.587, 0.539, 0.448, 0.404, 0.349, 0.313, 0.301, 0.312, 0.331, 0.347, 0.363, 0.375, 0.409, 0.399, 0.422, 0.429, 0.428, 0.454, 0.475, 0.469, 0.480, 0.493, 0.505, 0.486, 0.505, 0.505, 0.521, 0.533, 0.537, 0.550, 0.574, 0.581, 0.595, 0.658,
		4.194, 2.577, 1.939, 1.434, 1.128, 0.991, 0.842, 0.702, 0.615, 0.537, 0.505, 0.440, 0.409, 0.424, 0.444, 0.478, 0.520, 0.533, 0.561, 0.574, 0.594, 0.599, 0.646, 0.622, 0.617, 0.655, 0.679, 0.691, 0.688, 0.694, 0.711, 0.704, 0.717, 0.725, 0.762, 0.748, 0.747, 0.777, 0.810, 0.821, 0.859, 0.947,
		5.198, 3.061, 1.880, 1.400, 1.065, 0.912, 0.774, 0.662, 0.588, 0.530, 0.465, 0.431, 0.368, 0.357, 0.378, 0.417, 0.430, 0.463, 0.469, 0.494, 0.508, 0.553, 0.549, 0.549, 0.548, 0.585, 0.605, 0.597, 0.611, 0.609, 0.636, 0.629, 0.643, 0.647, 0.658, 0.670, 0.678, 0.692, 0.705, 0.729, 0.759, 0.838,
		5.938, 2.660, 2.003, 1.444, 1.131, 0.955, 0.777, 0.671, 0.582, 0.528, 0.440, 0.412, 0.344, 0.336, 0.359, 0.376, 0.397, 0.403, 0.420, 0.433, 0.445, 0.461, 0.472, 0.495, 0.467, 0.491, 0.488, 0.508, 0.498, 0.503, 0.530, 0.518, 0.525, 0.524, 0.528, 0.543, 0.548, 0.559, 0.571, 0.582, 0.603, 0.645,
		4.427, 2.716, 2.055, 1.477, 1.148, 0.942, 0.807, 0.710, 0.578, 0.532, 0.480, 0.431, 0.448, 0.463, 0.500, 0.531, 0.557, 0.592, 0.620, 0.629, 0.669, 0.661, 0.657, 0.636, 0.660, 0.702, 0.684, 0.711, 0.711, 0.718, 0.722, 0.753, 0.730, 0.752, 0.743, 0.767, 0.775, 0.790, 0.792, 0.833, 0.861, 0.930,
		4.374, 2.707, 1.873, 1.502, 1.142, 0.941, 0.756, 0.652, 0.585, 0.514, 0.465, 0.395, 0.399, 0.422, 0.446, 0.475, 0.497, 0.521, 0.534, 0.546, 0.575, 0.594, 0.584, 0.601, 0.597, 0.611, 0.617, 0.625, 0.620, 0.644, 0.640, 0.650, 0.659, 0.671, 0.662, 0.682, 0.687, 0.691, 0.722, 0.720, 0.754, 0.821,
		5.514, 2.799, 2.091, 1.483, 1.139, 0.949, 0.732, 0.652, 0.590, 0.526, 0.460, 0.395, 0.411, 0.411, 0.420, 0.439, 0.439, 0.468, 0.460, 0.461, 0.473, 0.496, 0.478, 0.489, 0.506, 0.512, 0.528, 0.507, 0.503, 0.527, 0.528, 0.539, 0.542, 0.545, 0.542, 0.562, 0.569, 0.558, 0.577, 0.590, 0.613, 0.640,
		5.076, 2.969, 2.168, 1.563, 1.156, 0.990, 0.789, 0.685, 0.591, 0.512, 0.479, 0.492, 0.497, 0.560, 0.577, 0.623, 0.603, 0.674, 0.683, 0.651, 0.705, 0.697, 0.719, 0.659, 0.720, 0.731, 0.755, 0.732, 0.758, 0.738, 0.756, 0.775, 0.764, 0.762, 0.780, 0.776, 0.779, 0.803, 0.805, 0.820, 0.847, 0.924,
		4.946, 3.074, 2.033, 1.526, 1.145, 0.925, 0.773, 0.679, 0.600, 0.497, 0.459, 0.444, 0.476, 0.498, 0.510, 0.528, 0.555, 0.569, 0.588, 0.593, 0.599, 0.595, 0.629, 0.628, 0.613, 0.626, 0.649, 0.654, 0.650, 0.662, 0.676, 0.668, 0.647, 0.661, 0.691, 0.688, 0.710, 0.713, 0.721, 0.729, 0.755, 0.800,
		4.379, 2.789, 2.046, 1.451, 1.204, 0.931, 0.801, 0.639, 0.593, 0.488, 0.454, 0.477, 0.477, 0.485, 0.484, 0.520, 0.518, 0.477, 0.492, 0.490, 0.525, 0.507, 0.555, 0.548, 0.546, 0.532, 0.529, 0.537, 0.548, 0.546, 0.552, 0.549, 0.556, 0.560, 0.587, 0.562, 0.568, 0.581, 0.587, 0.590, 0.605, 0.646,
		4.865, 3.154, 2.085, 1.565, 1.158, 0.974, 0.784, 0.677, 0.594, 0.534, 0.560, 0.575, 0.606, 0.625, 0.661, 0.685, 0.708, 0.718, 0.717, 0.708, 0.734, 0.734, 0.737, 0.717, 0.741, 0.741, 0.759, 0.769, 0.759, 0.772, 0.779, 0.781, 0.784, 0.760, 0.795, 0.775, 0.796, 0.795, 0.807, 0.828, 0.850, 0.899,
		4.710, 3.265, 2.242, 1.623, 1.245, 0.980, 0.788, 0.666, 0.565, 0.538, 0.534, 0.549, 0.569, 0.589, 0.592, 0.647, 0.612, 0.616, 0.648, 0.669, 0.654, 0.696, 0.661, 0.671, 0.670, 0.675, 0.671, 0.669, 0.676, 0.672, 0.686, 0.691, 0.701, 0.687, 0.707, 0.703, 0.721, 0.715, 0.714, 0.726, 0.744, 0.787,
		4.722, 3.204, 2.259, 1.530, 1.295, 0.990, 0.847, 0.694, 0.609, 0.594, 0.562, 0.563, 0.552, 0.570, 0.560, 0.570, 0.586, 0.550, 0.586, 0.549, 0.568, 0.597, 0.547, 0.578, 0.605, 0.583, 0.583, 0.563, 0.564, 0.562, 0.578, 0.569, 0.548, 0.584, 0.583, 0.577, 0.566, 0.576, 0.575, 0.604, 0.601, 0.633,
		4.987, 3.797, 2.304, 1.719, 1.239, 1.000, 0.821, 0.687, 0.684, 0.733, 0.693, 0.728, 0.744, 0.766, 0.797, 0.788, 0.813, 0.774, 0.806, 0.791, 0.792, 0.783, 0.807, 0.811, 0.817, 0.789, 0.795, 0.798, 0.782, 0.800, 0.805, 0.807, 0.820, 0.793, 0.779, 0.826, 0.816, 0.804, 0.826, 0.827, 0.834, 0.880,
		5.131, 3.792, 2.329, 1.684, 1.300, 1.010, 0.884, 0.691, 0.637, 0.727, 0.696, 0.712, 0.755, 0.691, 0.723, 0.758, 0.707, 0.723, 0.713, 0.713, 0.722, 0.707, 0.705, 0.718, 0.690, 0.717, 0.719, 0.693, 0.708, 0.729, 0.725, 0.701, 0.720, 0.715, 0.695, 0.712, 0.720, 0.709, 0.721, 0.722, 0.738, 0.773,
		4.968, 3.556, 2.253, 1.833, 1.446, 1.044, 0.826, 0.826, 0.747, 0.757, 0.778, 0.776, 0.788, 0.679, 0.675, 0.722, 0.633, 0.665, 0.636, 0.611, 0.620, 0.624, 0.575, 0.631, 0.594, 0.627, 0.617, 0.591, 0.604, 0.601, 0.584, 0.575, 0.571, 0.570, 0.576, 0.584, 0.569, 0.573, 0.577, 0.603, 0.594, 0.625,
		5.198, 4.267, 3.271, 2.350, 1.804, 1.595, 1.387, 1.462, 1.443, 1.421, 1.376, 1.333, 1.312, 1.322, 1.273, 1.170, 1.121, 1.094, 1.070, 1.133, 1.065, 1.068, 1.001, 1.019, 0.941, 0.966, 0.907, 0.925, 0.841, 0.846, 0.873, 0.867, 0.867, 0.865, 0.802, 0.819, 0.843, 0.845, 0.821, 0.893, 0.850, 0.916,
		2.600, 4.364, 3.322, 2.423, 1.936, 1.601, 1.559, 1.507, 1.546, 1.357, 1.298, 1.317, 1.224, 1.183, 1.167, 1.104, 1.025, 1.006, 0.970, 0.990, 0.948, 0.892, 0.865, 0.900, 0.867, 0.832, 0.822, 0.773, 0.736, 0.760, 0.745, 0.708, 0.732, 0.734, 0.722, 0.716, 0.744, 0.750, 0.680, 0.709, 0.780, 0.789,
		4.050, 4.329, 3.231, 2.481, 2.209, 1.827, 1.686, 1.661, 1.573, 1.455, 1.311, 1.437, 1.394, 1.287, 1.138, 0.985, 1.001, 0.952, 0.933, 0.863, 0.766, 0.833, 0.782, 0.789, 0.692, 0.729, 0.650, 0.697, 0.689, 0.654, 0.671, 0.619, 0.570, 0.660, 0.606, 0.669, 0.630, 0.664, 0.642, 0.642, 0.641, 0.696
])
)

