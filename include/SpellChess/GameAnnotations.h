#pragma once


#include <vector>
#include <list>
#include <string>
#include <variant>
#include <map>
#include <functional>
#include "Board.h"

namespace SpellChessLib {
	
	/*
		Annotation namespace consists of utility class and functions involved with annotated games. 
		The indented use cases of these include:
		- Providing an intermediate game representation before its serialization
		- Providing a straightforward abstraction for GUIs to access move annotations via NAG values
		- Providing a straightforward abstraction for GUIs to access alternative variations

		The code structure was designed to match pgn formal grammar, which makes it easy to export the resulting code representation to pgn format.
	*/
	namespace Annotation {

		enum GameTermination {
			WhiteWon,
			BlackWon,
			Draw,
			Unknown
		};

		// planned to used by gui project, currently not integrated with game annotation system 
		enum MoveClassification {
			BookMove,
			Blunder,
			Mistake,
			Inaccuraccy,
			GoodMove,
			BrilliantMove
		};

		class MoveIndication {
			unsigned int value;
		public:
			MoveIndication(int nr) : value(nr) {};
			const unsigned int& getValue() const;
		};

		class NumericAnnotationGlyph {
			unsigned int glyphCode;
		public:
			NumericAnnotationGlyph(int glyphCode) : glyphCode(glyphCode) {};
			const unsigned int& getGlyphCode() const;

			bool operator==(const NumericAnnotationGlyph & other) const;
			bool operator!=(const NumericAnnotationGlyph & other) const;

		};

		using AnnotatedSequenceElement = typename std::variant<std::string, NumericAnnotationGlyph, SpellChessMove>;
		
		struct TagPair {
			std::string name;
			std::string value;
		};
		
		struct SevenTagRoster {
			std::string event;
			std::string site;
			std::string date;
			std::string round;
			std::string white;
			std::string black;
			std::string result;
		};

		class TagSection {
			std::vector<TagPair> tagPairs;
		public:
			using iterator = typename std::vector<TagPair>::iterator;
			using const_iterator = typename std::vector<TagPair>::const_iterator;
			
			TagSection() = default;
			TagSection(const std::vector<TagPair> & pairs);
			TagSection(const TagSection & other);
			TagSection(const SevenTagRoster & roster);

			iterator begin();
			iterator end();
			const_iterator begin() const { return cbegin(); };
			const_iterator end() const { return cend(); };
			const_iterator cbegin() const;
			const_iterator cend() const;
			iterator find(const std::string& tagName);
			const_iterator find(const std::string& tagName) const;
			void push_back(const TagPair& tagPair);
			void emplace_back(const std::string& tagName, const std::string& value);
			std::string& operator[] (const std::string& tagName);
			bool operator==(const TagSection & other) const;
			bool operator!=(const TagSection & other) const;

		};

		class AnnotatedSequence;

		namespace Contants {
			enum class AnnotatedSequenceNodeType : size_t{
				Variation = 0,
				Element = 1
			};

			enum class AnnotatedSequenceItemType : size_t {
				Comment = 0,
				NumericAnnotationGlyph = 1,
				Move = 2
			};
		};

		

		using AnnotatedSequenceNode = typename std::variant<AnnotatedSequence, AnnotatedSequenceElement>;


		class AnnotatedSequence {
		public:
			using iterator = typename std::list<AnnotatedSequenceNode>::iterator;
			using const_iterator = typename std::list<AnnotatedSequenceNode>::const_iterator;
		private:
			std::list<AnnotatedSequenceNode> nodes;
			void deepcopy(const AnnotatedSequence & other);
		public:
			AnnotatedSequence();
			AnnotatedSequence(const AnnotatedSequence & other); // deepcopy

			iterator begin();
			iterator end();
			const_iterator begin() const;
			const_iterator end() const;
			const_iterator cbegin() const;
			const_iterator cend() const;

			void insertVariationAfter(iterator pos, AnnotatedSequence&& variation);
			void insertAnnotationGlyphAfter(iterator pos, const NumericAnnotationGlyph& glyph);
			void insertCommentAfter(iterator pos, const std::string& comment);

			void appendVariation(AnnotatedSequence && variation);
			void appendGlyph(const NumericAnnotationGlyph& glyph);
			void appendMove(const SpellChessMove& move);
			void appendComment(const std::string& comment);

			AnnotatedSequence & operator=(const AnnotatedSequence & other);
			bool operator==(const AnnotatedSequence & other) const;
			bool operator!=(const AnnotatedSequence & other) const;
		};

		struct GameAnnotations {
			TagSection tagSection;
			AnnotatedSequence annotatedSequence;
			GameTermination terminationReason;
			GameState root;
		};

		extern void validateAnnotatedSequenceMoveLegality(const GameState& root, const Annotation::AnnotatedSequence & seq);

		extern SevenTagRoster beginSevenTagRooster(const std::string& event, const std::string& site, const std::string& round);
		extern SevenTagRoster makePlaceHolderRoster();
		extern void addPlayerTagsToRoster(SevenTagRoster& roster, const std::string& whitePlayer, const std::string& blackPlayer);
		
		extern std::string makeCurrentDateTag();
		extern std::string makeResultTag(const GameTermination& reason);

		extern std::string makeIllegalMoveComment(const PlayerColor& color, const SpellChessMove & move);
		extern void exportAnnotatedGame(const std::string& filePath, const GameAnnotations& annotations);
	}

}