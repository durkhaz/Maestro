#pragma once

class ANGLES
{
public:
	float pitch, yaw;
	bool bDirty;
	ANGLES() { }
	ANGLES(float pitch_, float yaw_)
		: pitch(std::move(pitch_)), yaw(std::move(yaw_))
	{
		bDirty = false;
	}
	ANGLES(float pitch_, float yaw_, bool bDirty_)
		: pitch(std::move(pitch_)), yaw(std::move(yaw_)), bDirty(std::move(bDirty_))
	{
	}
};

// Pitch from 24 - 88
std::vector<ANGLES> PianoKeyAngles = {
	ANGLES(2.6f, -50.2f),
	ANGLES(-3.1f, -45.f),		//
	ANGLES(3.2f, -47.6f),
	ANGLES(-2.9f, -43.f),		//
	ANGLES(3.8f, -45.f),
	ANGLES(3.8f, -42.f),
	ANGLES(-2.2f, -38.3f),		//
	ANGLES(4.4f, -39.4f),
	ANGLES(-2.2f, -36.f),		//
	ANGLES(4.6f, -36.4f),
	ANGLES(-2.f, -33.5f),		//
	ANGLES(5.f, -33.6f),
	ANGLES(5.f, -30.6f),
	ANGLES(-1.8f, -28.6f),		//
	ANGLES(5.f, -27.5f),
	ANGLES(-1.8f, -26.5f),		//
	ANGLES(5.f, -24.4f),
	ANGLES(4.6f, -21.5f),
	ANGLES(-2.f, -21.5f),		//
	ANGLES(4.6f, -18.6f),
	ANGLES(-2.1f, -19.2f),		//
	ANGLES(4.6f, -15.5f),
	ANGLES(-2.2f, -16.9f),		//
	ANGLES(4.f, -12.8f),
	ANGLES(3.8f, -10.f),
	ANGLES(-2.8f, -12.1f),		//
	ANGLES(3.f, -7.3f),
	ANGLES(-3.f, -10.f),		//
	ANGLES(2.4f, -4.9f),
	ANGLES(2.f, -2.3f),
	ANGLES(-3.8f, -6.f),		//
	ANGLES(1.5f, 0.f),
	ANGLES(-4.2f, -3.8f),		//
	ANGLES(0.7f, 2.3f),
	ANGLES(-4.6f, -2.f),		//
	ANGLES(0.f, 4.4f),
	ANGLES(-0.5f, 6.7f),
	ANGLES(-5.6f, 2.f),			//
	ANGLES(-1.f, 9.f),
	ANGLES(-6.2f, 4.f),			//
	ANGLES(-1.8f, 10.f),
	ANGLES(-2.6f, 12.f),
	ANGLES(-7.2f, 7.2f),		//
	ANGLES(-3.f, 14.f),
	ANGLES(-8.f, 9.f),			//
	ANGLES(-4.f, 15.f),
	ANGLES(-8.5f, 10.6f),		//
	ANGLES(-4.4f, 17.f),
	ANGLES(-5.2f, 18.f),
	ANGLES(-9.5f, 13.3f),		//
	ANGLES(-6.f, 19.f),
	ANGLES(-10.f, 14.7f),		//
	ANGLES(-6.6f, 21.f),
	ANGLES(-7.4f, 22.6f),
	ANGLES(-11.1f, 17.2f),		//
	ANGLES(-8.f, 24.f),
	ANGLES(-11.8f, 18.5f),		//
	ANGLES(-8.6f, 25.f),
	ANGLES(-12.3f, 19.8f),		//
	ANGLES(-9.4f, 26.2f),
	ANGLES(-10.f, 27.7f),
	ANGLES(-13.5f, 22.f),		//
	ANGLES(-10.7f, 28.5f),
	ANGLES(-14.f, 23.f),		//
	ANGLES(-11.5f, 29.f),
};