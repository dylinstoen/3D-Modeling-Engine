#version 400

out vec4 frag_color;

in vec4 interpolated_position;

uniform int object_id;

const int FLOAT_SIG_BITS = 23;
const int FLOAT_EXP_MASK = 1 + (1 << FLOAT_SIG_BITS);


void main()
{
    float encoded_id = intBitsToFloat(object_id + FLOAT_EXP_MASK);
    frag_color = vec4(interpolated_position.xyz/interpolated_position.w, encoded_id);
}
