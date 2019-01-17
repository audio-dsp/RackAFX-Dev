#define HALF_PI 1.5707963267948966192313216916398f

//----------------------------------------------------------------------
float FastSin0 (float fAngle)
{
    float fASqr = fAngle*fAngle;
    float fResult = 7.61e-03f;
    fResult *= fASqr;
    fResult -= 1.6605e-01f;
    fResult *= fASqr;
    fResult += 1.0f;
    fResult *= fAngle;
    return fResult;
}
//----------------------------------------------------------------------
float FastSin1 (float fAngle)
{
    float fASqr = fAngle*fAngle;
    float fResult = -2.39e-08f;
    fResult *= fASqr;
    fResult += 2.7526e-06f;
    fResult *= fASqr;
    fResult -= 1.98409e-04f;
    fResult *= fASqr;
    fResult += 8.3333315e-03f;
    fResult *= fASqr;
    fResult -= 1.666666664e-01f;
    fResult *= fASqr;
    fResult += 1.0f;
    fResult *= fAngle;
    return fResult;
}
//----------------------------------------------------------------------
float FastCos0 (float fAngle)
{
    float fASqr = fAngle*fAngle;
    float fResult = 3.705e-02f;
    fResult *= fASqr;
    fResult -= 4.967e-01f;
    fResult *= fASqr;
    fResult += 1.0f;
    return fResult;
}
//----------------------------------------------------------------------
float FastCos1 (float fAngle)
{
    float fASqr = fAngle*fAngle;
    float fResult = -2.605e-07f;
    fResult *= fASqr;
    fResult += 2.47609e-05f;
    fResult *= fASqr;
    fResult -= 1.3888397e-03f;
    fResult *= fASqr;
    fResult += 4.16666418e-02f;
    fResult *= fASqr;
    fResult -= 4.999999963e-01f;
    fResult *= fASqr;
    fResult += 1.0f;
    return fResult;
}
//----------------------------------------------------------------------
float FastTan0 (float fAngle)
{
    float fASqr = fAngle*fAngle;
    float fResult = 2.033e-01f;
    fResult *= fASqr;
    fResult += 3.1755e-01f;
    fResult *= fASqr;
    fResult += 1.0f;
    fResult *= fAngle;
    return fResult;
}
//----------------------------------------------------------------------
float FastTan1 (float fAngle)
{
    float fASqr = fAngle*fAngle;
    float fResult = 9.5168091e-03f;
    fResult *= fASqr;
    fResult += 2.900525e-03f;
    fResult *= fASqr;
    fResult += 2.45650893e-02f;
    fResult *= fASqr;
    fResult += 5.33740603e-02f;
    fResult *= fASqr;
    fResult += 1.333923995e-01f;
    fResult *= fASqr;
    fResult += 3.333314036e-01f;
    fResult *= fASqr;
    fResult += 1.0f;
    fResult *= fAngle;
    return fResult;
}
//----------------------------------------------------------------------
float FastInvSin (float fValue)
{
    float fRoot = sqrt(1.0f-fValue);
    float fResult = -0.0187293f;
    fResult *= fValue;
    fResult += 0.0742610f;
    fResult *= fValue;
    fResult -= 0.2121144f;
    fResult *= fValue;
    fResult += 1.5707288f;
    fResult = HALF_PI - fRoot*fResult;
    return fResult;
}
//----------------------------------------------------------------------
float FastInvCos (float fValue)
{
    float fRoot = sqrt(1.0f-fValue);
    float fResult = -0.0187293f;
    fResult *= fValue;
    fResult += 0.0742610f;
    fResult *= fValue;
    fResult -= 0.2121144f;
    fResult *= fValue;
    fResult += 1.5707288f;
    fResult *= fRoot;
    return fResult;
}
//----------------------------------------------------------------------
float FastInvTan0 (float fValue)
{
    float fVSqr = fValue*fValue;
    float fResult = 0.0208351f;
    fResult *= fVSqr;
    fResult -= 0.085133f;
    fResult *= fVSqr;
    fResult += 0.180141f;
    fResult *= fVSqr;
    fResult -= 0.3302995f;
    fResult *= fVSqr;
    fResult += 0.999866f;
    fResult *= fValue;
    return fResult;
}
//----------------------------------------------------------------------
float FastInvTan1 (float fValue)
{
    float fVSqr = fValue*fValue;
    float fResult = 0.0028662257f;
    fResult *= fVSqr;
    fResult -= 0.0161657367f;
    fResult *= fVSqr;
    fResult += 0.0429096138f;
    fResult *= fVSqr;
    fResult -= 0.0752896400f;
    fResult *= fVSqr;
    fResult += 0.1065626393f;
    fResult *= fVSqr;
    fResult -= 0.1420889944f;
    fResult *= fVSqr;
    fResult += 0.1999355085f;
    fResult *= fVSqr;
    fResult -= 0.3333314528f;
    fResult *= fVSqr;
    fResult += 1.0f;
    fResult *= fValue;
    return fResult;
}
//----------------------------------------------------------------------
