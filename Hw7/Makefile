CC = gcc
TARGET = exec
CFLAGS = -fopenmp -lwiringPi -mfpu=neon -march=native


$(TARGET) : 20220900_opti.c
	$(CC) -o $(TARGET) 20220900_opti.c -lm $(CFLAGS)

/*
void Padding(float *feature_in, float *feature_out, int C, int H, int W) {
    asm volatile(
        "push {r4-r6, r8-r10, lr}\n\t"  // Save callee-saved registers

        // Load parameters into registers
        "mov r4, %[feature_in]\n\t"  // r4 = feature_in
        "mov r5, %[feature_out]\n\t" // r5 = feature_out
        "mov r6, %[C]\n\t"           // r6 = C
        "mov r8, %[H]\n\t"           // r8 = H
        "mov r9, %[W]\n\t"           // r9 = W

        // Calculate padded_H and padded_W
        "add r10, r8, #2\n\t"   // r10 = padded_H = H + 2
        "add r0, r9, #2\n\t"    // r0 = padded_W = W + 2

        // Outer loop for channels (c)
        "mov r1, #0\n\t"        // r1 = c = 0
        "c_loop:\n\t"
        "cmp r1, r6\n\t"        // if c >= C, break
        "bge c_done\n\t"

        // Inner loop for rows (h)
        "mov r2, #0\n\t"        // r2 = h = 0
        "h_loop:\n\t"
        "cmp r2, r10\n\t"       // if h >= padded_H, break
        "bge h_done\n\t"

        // Inner loop for columns (w)
        "mov r3, #0\n\t"        // r3 = w = 0
        "w_loop:\n\t"
        "cmp r3, r0\n\t"        // if w >= padded_W, break
        "bge w_done\n\t"

        // Calculate index for feature_out
        "mul r12, r1, r10\n\t"   // r12 = c * padded_H
        "mul r12, r12, r0\n\t"    // r12 = c * padded_H * padded_W
        "add r12, r12, r2\n\t"    // r12 += h
        "mul r12, r12, r0\n\t"    // r12 *= padded_W
        "add r12, r12, r3\n\t"    // r12 += w
        "lsl r12, r12, #2\n\t"    // r12 *= 4 (sizeof(float))

        // Check if we are on the border
        "cmp r2, #0\n\t"
        "beq zero_pad\n\t"
        "cmp r2, r10\n\t"
        "sub r11, r10, #1\n\t"
        "beq zero_pad\n\t"
        "cmp r3, #0\n\t"
        "beq zero_pad\n\t"
        "cmp r3, r0\n\t"
        "sub r11, r0, #1\n\t"
        "beq zero_pad\n\t"

        // Not on the border, copy from feature_in
        // Calculate index for feature_in
        "sub r11, r2, #1\n\t"  // r11 = h - 1
        "mul r11, r11, r9\n\t" // r11 *= W
        "sub r10, r3, #1\n\t"  // r10 = w - 1
        "add r11, r11, r10\n\t"// r11 += w - 1
        "lsl r11, r11, #2\n\t" // r11 *= 4 (sizeof(float))
        "add r11, r11, r4\n\t" // r11 += feature_in
        "ldr r11, [r11]\n\t"   // r11 = feature_in[c * H * W + (h - 1) * W + (w - 1)]
        "b store\n\t"

        "zero_pad:\n\t"
        "mov r11, #0\n\t"  // r11 = 0

        "store:\n\t"
        "add r10, r5, r12\n\t"  // r10 = &feature_out[c * padded_H * padded_W + h * padded_W + w]
        "str r11, [r10]\n\t"   // feature_out[...] = r11

        "add r3, r3, #1\n\t"   // w++
        "b w_loop\n\t"

        "w_done:\n\t"
        "add r2, r2, #1\n\t"   // h++
        "b h_loop\n\t"

        "h_done:\n\t"
        "add r1, r1, #1\n\t"   // c++
        "b c_loop\n\t"

        "c_done:\n\t"
        // Restore callee-saved registers and return
        "pop {r4-r6, r8-r10, lr}\n\t"
        "bx lr\n\t"
        :
        : [feature_in] "r"(feature_in), [feature_out] "r"(feature_out), [C] "r"(C), [H] "r"(H), [W] "r"(W)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "r12", "memory"
    );
}
*/
