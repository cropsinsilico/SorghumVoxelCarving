#include "MainWindow.h"

#include "ConsoleApplication.h"

#include <QtWidgets/QApplication>
#include <QCommandLineParser>
#include <QSurfaceFormat>

enum class CommandLineParseResult
{
	OkGui,
	OkCmd,
	Error
};

CommandLineParseResult parseCommandLine(const QApplication& app,
	                                    QCommandLineParser& parser,
	                                    ConsoleApplicationParameters* parameters);

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// Parse command line arguments
	QCommandLineParser parser;
	parser.setApplicationDescription("Automatic reconstruction of Sorghum plants from multiple RGB images");
	ConsoleApplicationParameters parameters;
	const CommandLineParseResult action = parseCommandLine(app, parser, &parameters);

	if (action == CommandLineParseResult::OkGui)
	{
		QSurfaceFormat format;
		format.setDepthBufferSize(24);
		format.setSamples(4);
		format.setProfile(QSurfaceFormat::CoreProfile);
		format.setOption(QSurfaceFormat::DebugContext);
		QSurfaceFormat::setDefaultFormat(format);

		MainWindow w;
		w.show();
		return app.exec();
	}
	else if (action == CommandLineParseResult::OkCmd)
	{
		ConsoleApplication consoleApp(parameters);

		// Use a queued connection because the console app may emit the finished signal before app.exec() is called
		QObject::connect(&consoleApp, &ConsoleApplication::finished, &app, &QApplication::exit, Qt::QueuedConnection);

		consoleApp.exec();
		return app.exec();
	}
	else if (action == CommandLineParseResult::Error)
	{
		qWarning() << "Error while parsing the command line arguments.";
		parser.showHelp();
	}

	return 1;
}

CommandLineParseResult parseCommandLine(
	const QApplication& app,
	QCommandLineParser& parser,
	ConsoleApplicationParameters* parameters)
{
	const QCommandLineOption helpOption = parser.addHelpOption();

	// A boolean option to launch the application in a graphical interface
	const QCommandLineOption guiOption(
		QStringList() << "g" << "gui",
		QCoreApplication::translate("main", "Launch the application in a graphical interface."));
	parser.addOption(guiOption);

	// An option to set the command to execute
	const QCommandLineOption commandOption(
		QStringList() << "c" << "command",
		QCoreApplication::translate("main", "Command to execute: calibration, reconstruction"),
		QCoreApplication::translate("main", "command"));
	parser.addOption(commandOption);

	// An option to set the input file
	const QCommandLineOption inputOption(
		QStringList() << "i" << "input",
		QCoreApplication::translate("main", "Input image file."),
		QCoreApplication::translate("main", "file"));
	parser.addOption(inputOption);
	
	// An option to set the output file
	const QCommandLineOption outputOption(
		QStringList() << "o" << "output",
		QCoreApplication::translate("main", "Output image file."),
		QCoreApplication::translate("main", "file"));
	parser.addOption(outputOption);

	// Process the actual command line arguments given by the user
	parser.process(app);

	// (GUI option) or (no arguments at all), launch the GUI application
	if (parser.isSet(guiOption) || !parser.isSet(commandOption) || !parser.isSet(inputOption) || !parser.isSet(outputOption))
	{
		return CommandLineParseResult::OkGui;
	}
	
	// Command, input and output files set, launch the cmd application
	if (parser.isSet(commandOption) && parser.isSet(inputOption) && parser.isSet(outputOption))
	{
		parameters->commandType = readCommandTypeFromString(parser.value(commandOption).toStdString());
		parameters->inputFile = parser.value(inputOption);
		parameters->outputFile = parser.value(outputOption);
		return CommandLineParseResult::OkCmd;
	}

	return CommandLineParseResult::Error;
}