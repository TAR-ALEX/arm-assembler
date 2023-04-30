//
//  block_shift_immeditate.cpp
//  scat
//
//  Created by Alex Tarasov on 7/4/20.
//  Copyright © 2020 Alex Tarasov. All rights reserved.

#pragma once

#include <string>

using namespace std;


//returns {shift, immediate}
pair<string, string> block_shift_immeditate_Encode(long long targetValue, int shiftOptions, int blockSize);

//returns {negate, shift, immediate}
tuple<bool, string, string> block_shift_flip_immeditate_Encode(long long targetValue, int shiftOptions, int blockSize);
