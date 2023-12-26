#include "MLP.h"

double frand(){
	return (2.0*(double)rand() / RAND_MAX) - 1.0;
}

// Return a new Perceptron object with the specified number of inputs (+1 for the bias).
Perceptron::Perceptron(size_t inputs, double bias){
	this->bias = bias;
	weights.resize(inputs+1);
	generate(weights.begin(),weights.end(),frand);
}

// Run the perceptron. x is a vector with the input values.
double Perceptron::run(std::vector<double> x){
	x.push_back(bias);
	double sum = inner_product(x.begin(),x.end(),weights.begin(),(double)0.0);
	return sigmoid(sum);
}

// Set the weights. w_init is a vector with the weights.
void Perceptron::set_weights(std::vector<double> w_init){
	weights = w_init;
}

// Evaluate the sigmoid function for the floating point input x.
double Perceptron::sigmoid(double x){
	return 1.0/(1.0 + exp(-x));
}

// Return a new MultiLayerPerceptron object with the specified parameters.
MultiLayerPerceptron::MultiLayerPerceptron(std::vector<size_t> layers, double bias, double eta) {
    this->layers = layers;
    this->bias = bias;
    this->eta = eta;

    for (size_t i = 0; i < layers.size(); i++){
        values.push_back(std::vector<double>(layers[i],0.0));
        d.push_back(std::vector<double>(layers[i],0.0));
        network.push_back(std::vector<Perceptron>());
        if (i > 0)   //network[0] is the input layer,so it has no neurons
            for (size_t j = 0; j < layers[i]; j++)
                network[i].push_back(Perceptron(layers[i-1], bias));
    }
}

// Set the weights. w_init is a vector of vectors of vectors with the weights for all but the input layer.
void MultiLayerPerceptron::set_weights(std::vector<std::vector<std::vector<double> > > w_init) {
    for (size_t i = 0; i < w_init.size(); i++)
        for (size_t j = 0; j < w_init[i].size(); j++)
            network[i+1][j].set_weights(w_init[i][j]);
}

void MultiLayerPerceptron::print_weights() {
    std::cout << std::endl;
    for (size_t i = 1; i < network.size(); i++){
        for (size_t j = 0; j < layers[i]; j++) {
            std::cout << "Layer " << i+1 << " Neuron " << j << ": ";
            for (auto &it: network[i][j].weights)
                std::cout << it << "   ";
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

// Feed a sample x into the MultiLayer Perceptron.
std::vector<double> MultiLayerPerceptron::run(std::vector<double> x) {
    values[0] = x;
    for (size_t i = 1; i < network.size(); i++)
        for (size_t j = 0; j < layers[i]; j++)
            values[i][j] = network[i][j].run(values[i-1]);
    return values.back();
}

// Run a single (x,y) pair with the backpropagation algorithm.
// x is the input value, y is output
double MultiLayerPerceptron::bp(std::vector<double> x, std::vector<double> y){
    // Feeding a sample to the network
    std::vector<double> outputs = run(x);

    // Calculating the Mean Squared Error
    std::vector<double> error;
    double MSE{0.0};
    for (unsigned int i{0}; i < outputs.size(); i++) {
        error.emplace_back(y[i] - outputs[i]);
        MSE += error[i] * error[i];
    }
    MSE /= layers.back();
    
    // Calculating the output error terms
    for (unsigned int i{0}; i < outputs.size(); i--) {
        d.back()[i] = outputs[i] * (1 - outputs[i]) * error[i];
    }
    // Calculating the error term of each unit on each layer    
    for (size_t i = network.size()-2; i > 0; i--)
        for (size_t h = 0; h < network[i].size(); h++){
            double fwd_error = 0.0;
            for (size_t k = 0; k < layers[i+1]; k++)
                fwd_error += network[i+1][k].weights[h] * d[i+1][k];
            d[i][h] = values[i][h] * (1-values[i][h]) * fwd_error;
        }
    
    // Calculating the deltas and update the weights
    for (size_t i = 1; i < network.size(); i++)
        for (size_t j = 0; j < layers[i]; j++)
            for (size_t k = 0; k < layers[i-1]+1; k++){
                double delta;
                if (k == layers[i-1])
                    delta = eta * d[i][j] * bias;
                else
                    delta = eta * d[i][j] * values[i-1][k];
                network[i][j].weights[k] += delta;
            }
    return MSE;
}