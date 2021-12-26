#include "daScript/misc/platform.h"
#include "daScript/ast/ast.h"
#include "daScript/ast/ast_interop.h"
#include "daScript/ast/ast_handle.h"
#include "daScript/ast/ast_typefactory_bind.h"
#include "daScript/simulate/bind_enum.h"
#include "dasSFML.h"
#include "need_dasSFML.h"
#include "aot_dasSFML.h"

IMPLEMENT_EXTERNAL_TYPE_FACTORY(Bvec2,sf::Glsl::Bvec2);
IMPLEMENT_EXTERNAL_TYPE_FACTORY(Bvec3,sf::Glsl::Bvec3);
IMPLEMENT_EXTERNAL_TYPE_FACTORY(Bvec4,sf::Glsl::Bvec4);

IMPLEMENT_EXTERNAL_TYPE_FACTORY(Mat3,sf::Glsl::Mat3);
IMPLEMENT_EXTERNAL_TYPE_FACTORY(Mat4,sf::Glsl::Mat4);


namespace das {

    void sfml_window_close ( sf::Window & win ) { win.close(); }

    void sflm_with_render_target ( sf::RenderWindow & win, const TBlock<void,sf::RenderTarget> & block, Context * context, LineInfoArg * at ) {
        das_invoke<void>::invoke<sf::RenderTarget&>(context,at,block,win);
    }

    const sf::RenderStates & sfml_render_states_default() {
        return sf::RenderStates::Default;
    }

    void sfml_with_transformable ( sf::Shape & shape, const TBlock<void,sf::Transformable> & block, Context * context, LineInfoArg * at ) {
        das_invoke<void>::invoke<sf::Transformable&>(context,at,block,shape);
    }

    void Module_dasSFML::initAotAlias () {
        addAlias(typeFactory<sf::Vector2f>::make(lib));
        addAlias(typeFactory<sf::Vector2i>::make(lib));
        addAlias(typeFactory<sf::Vector2u>::make(lib));
        addAlias(typeFactory<sf::IntRect>::make(lib));
        addAlias(typeFactory<sf::FloatRect>::make(lib));
        addAnnotation(make_smart<DummyTypeAnnotation>("Bvec2", "sf::Glsl::Bvec2", sizeof(sf::Glsl::Bvec2), alignof(sf::Glsl::Bvec2)));
        addAnnotation(make_smart<DummyTypeAnnotation>("Bvec3", "sf::Glsl::Bvec3", sizeof(sf::Glsl::Bvec3), alignof(sf::Glsl::Bvec3)));
        addAnnotation(make_smart<DummyTypeAnnotation>("Bvec4", "sf::Glsl::Bvec4", sizeof(sf::Glsl::Bvec4), alignof(sf::Glsl::Bvec4)));
        addAnnotation(make_smart<DummyTypeAnnotation>("Mat3", "sf::Glsl::Mat3", sizeof(sf::Glsl::Mat3), alignof(sf::Glsl::Mat3)));
        addAnnotation(make_smart<DummyTypeAnnotation>("Mat4", "sf::Glsl::Mat4", sizeof(sf::Glsl::Mat4), alignof(sf::Glsl::Mat4)));
    }

	void Module_dasSFML::initMain () {
        // sf::Color
        addCtor<sf::Color>(*this,lib,"Color","sf::Color");
        addCtor<sf::Color,uint32_t>(*this,lib,"Color","sf::Color");
        addCtor<sf::Color,uint8_t,uint8_t,uint8_t,uint8_t>(*this,lib,"Color","sf::Color")
            ->arg_init(3,make_smart<ExprConstUInt>(255));
        // sf::ContextSettings
        addCtorAndUsing<sf::ContextSettings>(*this,lib,"ContextSettings","sf::ContextSettings");
        // sf::VideoMode
        addCtorAndUsing<sf::VideoMode>(*this,lib,"VideoMode","sf::VideoMode");
        addCtor<sf::VideoMode,uint32_t,uint32_t,uint32_t>(*this,lib,"VideoMode","sf::VideoMode")
            ->arg_init(2,make_smart<ExprConstUInt>(24));
        // sf::Window
        addCtorAndUsing<sf::Window>(*this,lib,"Window","sf::Window");
        addCtorAndUsing<sf::Window,sf::VideoMode,char *,uint32_t,const sf::ContextSettings &>(*this,lib,"Window","sf::Window");
        addExtern<DAS_BIND_FUN(sfml_window_close)>(*this,lib,"close",
            SideEffects::worstDefault,"sfml_window_close");
        // sf::RenderWindow
        addCtorAndUsing<sf::RenderWindow>(*this,lib,"RenderWindow","sf::RenderWindow");
        addCtorAndUsing<sf::RenderWindow,sf::VideoMode,char *,uint32_t,const sf::ContextSettings &>(*this,lib,"RenderWindow","sf::RenderWindow");
        addExtern<DAS_BIND_FUN(sflm_with_render_target)>(*this,lib,"with_render_target",
            SideEffects::invoke,"sflm_with_render_target");
        // sf::Shape
        addExtern<DAS_BIND_FUN(sfml_with_transformable)>(*this,lib,"with_transformable",
            SideEffects::invoke,"sfml_with_transformable");
        // sf::CircleShape
        addCtor<sf::CircleShape>(*this,lib,"CircleShape","sf::CircleShape");
        addCtor<sf::CircleShape,float>(*this,lib,"CircleShape","sf::CircleShape");
        // render states
        addExtern<DAS_BIND_FUN(sfml_render_states_default)>(*this,lib,"RenderStates_Default",
            SideEffects::invoke,"sfml_render_states_default");
        // time to fix-up const & Vector2i, Vector2u, Vector2f, and String
        for ( auto & pfn : this->functions.each() ) {
            for ( auto & arg : pfn->arguments ) {
                if ( arg->type->constant && arg->type->ref && arg->type->dim.size()==0 ) {
                    if ( arg->type->baseType==Type::tFloat2 || arg->type->baseType==Type::tInt2
                        || arg->type->baseType==Type::tUInt2 || arg->type->baseType==Type::tString ) {
                        arg->type->ref = false;
                    }
                }
            }
        }
        // fixup anything 'string' to need a string cast
        for ( auto & pfn : this->functions.each() ) {
            bool anyString = false;
            for ( auto & arg : pfn->arguments ) {
                if ( arg->type->isString() && !arg->type->ref ) {
                    anyString = true;
                }
            }
            if ( anyString ) {
                pfn->needStringCast = true;
            }
        }
        // if it takes uint8, i takes uint
        for ( auto & pfn : this->functions.each() ) {
            for ( auto & arg : pfn->arguments ) {
                if ( arg->type->baseType==Type::tUInt8 && arg->type->dim.size()==0 && !arg->type->ref ) {
                    arg->type->baseType = Type::tUInt;
                }
            }
        }
    }

	ModuleAotType Module_dasSFML::aotRequire ( TextWriter & tw ) const {
        // add your stuff here
        tw << "#include \"../modules/dasGlfw/src/sfml_stub.h\"\n";
        tw << "#include \"../modules/dasGlfw/src/aot_dasSFML.h\"\n";
        tw << "#include \"daScript/simulate/bind_enum.h\"\n";
        tw << "#include \"../modules/dasGlfw/src/dasSFML.enum.decl.cast.inc\"\n";
        return ModuleAotType::cpp;
    }

}