#pragma once

#include <vector>
#include <QString>
#include <QUrl>

namespace SpriteSet {

   class SpriteSetScheme {
      public:

      struct SpriteDef {
         QString spriteIdentifier;
         QUrl url;
      };

      protected:
         std::vector<SpriteDef> defs; 
      public:
      SpriteSetScheme() = default;
      SpriteSetScheme(const std::vector<SpriteDef> & definitions);
      virtual ~SpriteSetScheme() = default;

      void setDefinitions(const std::vector<SpriteDef> & definitions);
      void copyFromOther(const SpriteSetScheme & other);
      const std::vector<SpriteDef> & getDefinitions() const;
      const QUrl * findSpriteUrl(const QString & spriteIdentifier) const;
      constexpr bool validateOther(const SpriteSetScheme & other) const;
      constexpr bool isEmpty() const;
   };

};

constexpr bool SpriteSet::SpriteSetScheme::validateOther(const SpriteSetScheme & other) const{
    if (defs.size() != other.defs.size())
        return false;

    for(size_t i = 0; i < defs.size(); ++i ) 
        if (defs[i].spriteIdentifier != other.defs[i].spriteIdentifier)
            return false;
    return true;
};

constexpr bool SpriteSet::SpriteSetScheme::isEmpty() const {
    return defs.empty();
};