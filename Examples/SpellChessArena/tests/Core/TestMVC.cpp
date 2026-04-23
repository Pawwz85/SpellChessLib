#include <gtest/gtest.h>
#include <Core/MVC.h>


class CustomModel : public Core::Observable<int> {
    public:
    CustomModel(int v = 0) {
        state = v;
    };

    void set(int v) {
        state = v;
        notifyAll();
    }
};

class CustomController {
    protected:
    std::shared_ptr<CustomModel> model; 
    public:
    CustomController(std::shared_ptr<CustomModel> model) : model(model) {};

    virtual void methodA() {
        model->set(model->getState() + 1);
    }

    virtual void methodB() {
        model->set(model->getState() - 1);
    }
};

class DerivedController: public CustomController {
    public:
    DerivedController(std::shared_ptr<CustomModel> model) : CustomController(model) {};

    void methodA() override {
        model->set(model->getState() + 10);
    }

    virtual void methodB() override {
        model->set(model->getState() - 10);
    }
};

COMPONENT_PROTOTYPE(CustomComponent, CustomModel, CustomController) {
    public:
    std::shared_ptr<CustomController> makeOtherController(std::shared_ptr<CustomModel> model) const {
        return std::make_shared<DerivedController>(model);
    };
};

TEST(CustumComponent, test_model_factory) {
   std::shared_ptr<CustomModel> model = CustomComponent::getInstance().makeModel(7);
   ASSERT_EQ(model->getState(), 7);
}

TEST(CustumComponent, test_default_controller_factory) {
   std::shared_ptr<CustomModel> model = CustomComponent::getInstance().makeModel(7);
   auto controller = CustomComponent::getInstance().makeDefaultController(model);
   controller->methodA();
   ASSERT_EQ(model->getState(), 8);
}


TEST (CustomComponent, test_make_derived_controller) {
    auto model = CustomComponent::getInstance().makeModel(7);
    auto controller = CustomComponent::getInstance().makeOtherController(model);
    controller->methodA();
     ASSERT_EQ(model->getState(), 17);
}

