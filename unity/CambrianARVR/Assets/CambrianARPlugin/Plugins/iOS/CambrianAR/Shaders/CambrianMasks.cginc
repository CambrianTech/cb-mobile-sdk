
sampler2D m_maskY0;
sampler2D m_maskY1;
sampler2D m_maskY2;
sampler2D m_maskY3;
sampler2D m_maskY4;
sampler2D m_maskY5;

int m_numMasks;
float4 m_maskOffsets[10];
float4 m_lighting[10];
            
float getMaskValue(int index, float2 coordinate) {
    if (index == 0) {
        return tex2D(m_maskY0, coordinate).r;
    } 
    else if (index == 1) {
        return tex2D(m_maskY1, coordinate).r;
    }
    else if (index == 2) {
        return tex2D(m_maskY2, coordinate).r;
    }
    else if (index == 3) {
        return tex2D(m_maskY3, coordinate).r;
    }
    else if (index == 4) {
        return tex2D(m_maskY4, coordinate).r;
    }
    else if (index == 5) {
        return tex2D(m_maskY5, coordinate).r;
    }
    return 0.0;
}