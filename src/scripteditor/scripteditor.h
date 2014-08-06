#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QPlainTextEdit>
#include <QPoint>

#include "../vapoursynth/vsplugindata.h"

class QEvent;
class QKeyEvent;
class QResizeEvent;
class QPaintEvent;
class ScriptCompleterModel;
class ScriptCompleter;
class SyntaxHighlighter;

class ScriptEditor : public QPlainTextEdit
{
	Q_OBJECT

	public:

		ScriptEditor(QWidget * a_pParent = nullptr);

		virtual ~ScriptEditor();

		QString text();

		QPoint cursorPosition();

		void setCursorPosition(const QPoint & a_point);

		void setCursorPosition(int a_line, int a_index);

		bool isModified();

		void setModified(bool a_modified);

		void setPluginsList(const VSPluginsList & a_pluginsList);

		void setCharactersTypedToStartCompletion(int a_charactersNumber);

	public slots:

		void slotComplete();

		void slotInsertCompletion(const QString & a_completionString);

	protected:

		bool eventFilter(QObject * a_pObject, QEvent * a_pEvent);

		void resizeEvent(QResizeEvent * a_pEvent);

		void keyPressEvent(QKeyEvent * a_pEvent);

	private slots:

		void slotTextChanged();

		void slotUpdateSideBoxWidth();

		void slotUpdateSideBox(const QRect & a_rect, int a_dy);

		void slotHighlightCurrentBlock();

	private:

		QString getVapourSynthCoreName();

		void setChildrenCoreName(const QString & a_coreName);

		int sideBoxWidth();

		void paintSideBox(QPaintEvent * a_pEvent);

		void indentNewLine();

		QWidget * m_pSideBox;

		int m_sideBoxLineWidth;

		int m_sideBoxTextMargin;

		ScriptCompleterModel * m_pCompleterModel;

		ScriptCompleter * m_pCompleter;

		SyntaxHighlighter * m_pSyntaxHighlighter;

		int m_typedCharacters;

		int m_charactersTypedToStartCompletion;

		QString m_plainText;

};

#endif // SCRIPTEDITOR_H
