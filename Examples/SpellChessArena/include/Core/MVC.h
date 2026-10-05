#pragma once
#include <memory>
#include <vector>
#include "Singleton.h"
#include "Observer.h"

namespace Core {

    template <class State, class Controller>
    class View : public Observer<State> {
        protected:
        std::shared_ptr<Controller> currentController;

        public:
        virtual ~View() = default;

        void setController(std::shared_ptr<Controller> newController);
    };    

    /*!
        Component class stores a model and all valid controller that can manipulate said model.
    */
    template <class Model, class Controller> 
    class ComponentPrototype {
        public:
        virtual ~ComponentPrototype() = default;

        template<class ... Types>
        std::shared_ptr<Model> makeModel(Types... args) const {
            return std::make_shared<Model>(args...);
        }

        virtual std::shared_ptr<Controller> makeDefaultController(std::shared_ptr<Model> model) const = 0;
    };

    namespace Private {
       
        template <class Model, class Controller>
        class SingleControllerComponent : 
        public ComponentPrototype<Model, Controller>
        {
            public:
            std::shared_ptr<Controller> makeDefaultController(std::shared_ptr<Model> model) const override {
                return std::make_shared<Controller>(model);
            }
        };
        
    }

    template <class State, class Controller>
    void View<State, Controller>::setController(std::shared_ptr<Controller> controller) {
        currentController = controller;
    }

   // #define CONTROLLER_FACTORY(FactoryName, Model, Controller, Enumerator) std::shared_ptr<Model> FactoryName(std::shared_ptr<Model> model,  const Enumerator& enumerator)
   #define COMPONENT_PROTOTYPE(ComponentName, Model, Controller) class ComponentName : \
        public Core::Private::SingleControllerComponent<Model, Controller> , \
        public Core::Singleton<ComponentName> 
}