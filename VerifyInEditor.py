"""Run using Unreal Editor: Tools > Execute Python Script. Does not save test mutations."""
import unreal
from pathlib import Path

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level('/Game/Maps/smalltestmap')
cls = unreal.load_class(None, '/Script/SurvivalGame.ShelterSystem')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
managers = [actor for actor in actors if actor.get_class() == cls]
assert len(managers) == 1, 'Expected one saved building manager in smalltestmap'
result = managers[0].run_verification()
Path(unreal.Paths.project_dir(), 'VERIFICATION.txt').write_text(result)
unreal.log_warning(result)
assert result.startswith('Failures: 0'), result
