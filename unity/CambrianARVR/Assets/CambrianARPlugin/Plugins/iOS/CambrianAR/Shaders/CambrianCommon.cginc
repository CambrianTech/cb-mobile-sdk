
float4x4 m_displayTransform;
int m_isStillMode;

float2 getScreenPosition(float2 position)
{
    float texX = position.x;
    float texY = position.y;
        
    position.x = (m_displayTransform[0].x * texX + m_displayTransform[1].x * (texY) + m_displayTransform[2].x);
    position.y = (m_displayTransform[0].y * texX + m_displayTransform[1].y * (texY) + (m_displayTransform[2].y));
    
    return position;
}