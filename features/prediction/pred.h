#pragma once

class InputPrediction {
public:
	struct PredictionData {
		CUserCmd* m_last_cmd = nullptr;

		int m_tick = 0;
		int m_old_tick = 0;

		bool m_in_prediction = false;
		bool m_first_time_predicted = false;

		float m_curtime = 0.0f;
		float m_frametime = 0.0f;

		CMoveData data;

		int m_sequence_difference = 0;

		float m_weapon_spread = 0.0f;
		float m_weapon_inaccuracy = 0.0f;
		float m_weapon_range = 0.0f;
	};
private:
	PredictionData m_prediction_data;
public:
	void update( );
	void run( );
	void repredict( );
	void restore( );
};

extern InputPrediction g_inputpred;