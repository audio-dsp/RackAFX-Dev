#ifndef BASIC_BLOCKS_H
#define BASIC_BLOCKS_H
#include <vector>
using namespace std;

/*
 *
 * This is the base class, all blocks must derive from it
 *
 */

#pragma pack(1)

class BasicBlock {
public:
	BasicBlock(int numberOfInputs);
	void setInput(int i, BasicBlock * block);

	/*
	 * This method should return the next output (sample)
     * of the block.
	 */
	virtual float getNextValue() = 0;
	/*
	 * Reseta os parâmetros do bloco.
	 */
	virtual void resetBlock(void);
	/*
	 * Preenche o array output com amostras.
	 */
	virtual void generateSamples(float * output, int sampleNum);

	virtual void setON(bool ON);

	virtual bool getON();
protected:
	vector<BasicBlock *> inputs;
	bool ON;
};
/*
 * Somador
 *
 * Número de entradas: Configurável através do construtor
 */
class Adder: public BasicBlock {
public:
	Adder(int numberOfInputs);
	~Adder();

	float getNextValue();
};

class Multiplier: public BasicBlock {
public:
	Multiplier(int numberOfInputs);
	float getNextValue();
};

/*
 * Multiplexador
 *
 * Número de entradas: Configurável através do construtor
 */
class Multiplexer: public BasicBlock {
public:
	Multiplexer(int numberOfInputs, int selectedInput = 0);
	~Multiplexer();
	/*
	 * Seleciona a i-ésima entrada.
	 */
	void setSelectedInput(int i);
	int getSelectedInput();
	/*
	 * Obtém a saída do bloco selecionado
	 */
	float getNextValue();
private:
	int selectedInput;
};

/*
 * Uma constante numérica. Esse bloco não tem entradas, ele apenas
 * retorna um valor fixo ao chamar getNextValue
 *
 * Número de entradas: 0
 */
class Number: public BasicBlock {
public:
	Number(float number);
	~Number();
	float getNextValue();
	void setNumber(float number);
	float getNumber(void);
private:
	float number;
};

enum wavetypes {
	TRIG = 0, SAWTRIG, SAW, SQUARE, WIDERECT, NARROWRECT,
};

typedef enum wavetypes wavetype_t;

class Waveforms {
private:
	static vector<float> * trig;
	static vector<float> * sawtrig;
	static vector<float> * saw;
	static vector<float> * square;
	static vector<float> * widerect;
	static vector<float> * narrowrect;
public:
	static void initializeWaves(int length, int extraPoints);
	static void deleteWaves();
	static vector<float> * getWaveform(wavetype_t wave);
};

/*
 * Um oscilador de table lookup. É possível
 * configurar a forma de onda utilizada. Veja a enum
 * wavetypes e o método setWavetable
 *
 * Número de entradas: 2 (Frequência e Amplitude)
 */
class Oscil: public BasicBlock {
public:
	Oscil();
	Oscil(wavetype_t wave);

	void setFrequencyInput(BasicBlock * block);
	void setAmplitudeInput(BasicBlock * block);
	void setWavetable(wavetype_t wave);
	wavetype_t getWavetype();
	float getNextValue();
	void resetBlock(void);

	void setSlave(Oscil * block);
	void setSyncON(bool ON);
	bool getSyncON();
private:
	float cubic_interpolation(float phase) const;
	float linear_interpolation(float phase) const;
	vector<float> * table;
	float increment;
	float previous_phase;
	wavetype_t wave;

	Oscil * slave;
	double last_freq;
	int total_samples;
	int samples_til_reset;
	bool syncON;
};

enum noisetype {
	WHITE, PINK
};
typedef enum noisetype noisetype_t;

/*
 * Gerador de ruído branco ou rosa.
 *
 * Número de entradas: 0
 */
class Noise: public BasicBlock {
public:
	Noise();
	void setType(noisetype_t type);
	noisetype_t getType();
	float getNextValue();
	void setAmplitudeInput(BasicBlock * block);
private:
	noisetype_t noise_type;
	float (*noiseGenerator)();
};

enum phasetype {
	ATTACK, DECAY, SUSTAIN, RELEASE
};
typedef enum phasetype phase_t;

class ADSR: public BasicBlock {
public:
	ADSR();
	ADSR(float attack, float decay, float sustain, float release);
	void resetBlock();
	float getNextValue();

	void setPhase(phase_t phase);
	phase_t getPhase();

	void setAttack(float attack);
	float getAttack() const;
	void setDecay(float decay);
	float getDecay() const;
	void setSustain(float sustain);
	float getSustain() const;
	void setRelease(float release);
	float getRelease() const;

	void setSustain_amp(float sustain_amp);
	void setRelease_sharpness(float release_sharpness);
	void setDecay_sharpness(float decay_sharpness);
	void setAttack_sharpness(float attack_sharpness);
	void setReleaseON(bool on);

	float getSustain_amp() const;
	float getRelease_sharpness() const;
	float getDecay_sharpness() const;
	float getAttack_sharpness() const;
	bool getReleaseON() const;
private:

	/*
	 * Indica em qual fase a envoltória está atualmente
	 */
	phase_t current_phase;
	/*
	 * O número de amostras que já foram emitidas da fase atual
	 *
	 */
	int current_phase_samples;
	float attack;
	float decay;
	float sustain;
	float release;
	float attack_sharpness;
	float decay_sharpness;
	float sustain_amp;
	float release_sharpness;
	/*
	 * Indica se o release está ativo.
	 */
	bool releaseON;

	float attack_function();
	float decay_function();
	float release_function();
};

/*
 * Filtro do Moog, com frequencia controlada e fator de qualidade.
 *
 * Numero de entradas: 3, o sinal a ser filtrado e a frequencia de corte e o fator de qualidade k
 * 
 */
class Filter: public BasicBlock{
	public:
		Filter();
		virtual ~Filter();
		void setInputSignal(BasicBlock * block);
		void setFrequencyInput(BasicBlock * block);
		void setQualityInput(BasicBlock * block);
		float getNextValue();
	private:
		float x_1[4];
		float y_1[4];
			
};
#endif 
