#include "vgg16.hpp"

#include "config/click.hpp"

using namespace std;
using namespace wzp;


nn::Model get_model() {
    auto model = nn::Squential::CreateSquential();
    model->Add(Vgg());
    model->Add(nn::Softmax::CreateSoftmax());
    return model;
}


void train() {
    std::string input_dir = wzp::Click::get("--input_dir");
    // the parameters
    int batch_size = 256;
    int epoch_num = 100;
    float lr = 0.1;
    float weight_decay = 5e-4;

    int display = 50;

    // get model
    auto model = get_model();
    model->set_node_name("Cifar10Vgg13");

     // define the loss
    auto cross_entropy_loss = nn::CrossEntropyLoss::CreateCrossEntropyLoss();

    // define the optimizer
    auto optimizer = optim::SGD::CreateSGD(model->RegisterWeights(), lr, 0.9, weight_decay);

    io::Cifar10Dataset cifar10_train(input_dir, io::Cifar10Dataset::TRAIN);
    io::Cifar10Dataset cifar10_test(input_dir, io::Cifar10Dataset::TEST);
    cifar10_train.Load();
    cifar10_test.Load();

    // define a trainer to train the classification task
    auto trainer = utils::ClassifyTrainer::CreateClassfyTrainer(model, cross_entropy_loss, optimizer, cifar10_train,
        batch_size, epoch_num, display, &cifar10_test);

    // train
    trainer->Train();
    // save model
    io::Saver::Save("vgg.liconmodel", model);
}

void test() {
    std::string input_dir = wzp::Click::get("--input_dir");
    // get model
    auto model = get_model();
    model->set_phase(licon::Phase::TEST);

    // load model
    io::Saver::Load("vgg.liconmodel", &model);
    wzp::log::info("vgg model loaded...");

    io::Cifar10Dataset cifar10_test(input_dir, io::Cifar10Dataset::TEST);
    cifar10_test.Load();
    // eval
    wzp::log::info("The final accuracy is", utils::Accuracy::CreateAccuracy(model, cifar10_test)->Run());
}

int main(int argc, char *argv[])
{
    wzp::Click::add_argument("--mode", "the mode train or test");
    wzp::Click::add_argument("--input_dir", "the input dir of mnist root dir");
    wzp::Click::parse(argc, argv);

    // set up the env
    licon::EnvSetUp();

    wzp::log::info("Run VGG Quick...");

    if(wzp::Click::get("--mode") == "train") {
        train();
    } else {
        test();
    }
    return 0;
}

