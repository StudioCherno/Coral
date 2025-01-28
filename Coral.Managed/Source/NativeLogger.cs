//using Microsoft.Build.Evaluation;
//using Microsoft.Build.Execution;
//using Microsoft.Build.Framework;
//using Microsoft.Build.Utilities;

namespace Coral.Managed;

using static Coral.Managed.ManagedHost;

/*public class NativeLogger : Microsoft.Build.Utilities.Logger
{
    public override void Initialize(IEventSource src)
    {
        src.WarningRaised += new BuildWarningEventHandler((sender, args) => Base(args, MessageLevel.Warning, $"{args.File}({args.LineNumber}, {args.ColumnNumber})"));
        src.ErrorRaised += new BuildErrorEventHandler((sender, args) => Base(args, MessageLevel.Error, $"{args.File}({args.LineNumber}, {args.ColumnNumber})"));

        src.MessageRaised += new BuildMessageEventHandler(Message);

        src.TaskStarted += new TaskStartedEventHandler((sender, args) => Base(args, MessageLevel.Trace));
        src.ProjectStarted += new ProjectStartedEventHandler((sender, args) => Base(args, MessageLevel.Trace));
        src.ProjectFinished += new ProjectFinishedEventHandler((sender, args) => Base(args, MessageLevel.Trace));
    }

    private void Base(BuildEventArgs InArgs, MessageLevel InLevel, string InSuffix = "")
    {
		LogMessage($"[{InArgs.SenderName}] {InArgs.Message} {InSuffix}", InLevel);
    }

	private bool CheckLevel(BuildMessageEventArgs args, MessageImportance level, LoggerVerbosity verbosity)
	{
		return args.Importance == level && IsVerbosityAtLeast(verbosity);
	}

    void Message(object sender, BuildMessageEventArgs args)
    {
		if (CheckLevel(args, MessageImportance.High, LoggerVerbosity.Minimal) ||
			CheckLevel(args, MessageImportance.Normal, LoggerVerbosity.Normal) ||
			CheckLevel(args, MessageImportance.Low, LoggerVerbosity.Detailed))
        {
			Base(args, MessageLevel.Warning);
        }
    }
}
*/
