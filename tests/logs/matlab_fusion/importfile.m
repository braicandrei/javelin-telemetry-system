function data = importfile(filename, dataLines)
%IMPORTFILE Import data from a text file
%  DATA = IMPORTFILE(FILENAME) reads data from text file FILENAME for
%  the default selection.  Returns the data as a table.
%
%  DATA = IMPORTFILE(FILE, DATALINES) reads data for the specified row
%  interval(s) of text file FILENAME. Specify DATALINES as a positive
%  scalar integer or a N-by-2 array of positive scalar integers for
%  dis-contiguous row intervals.
%
%  Example:
%  data = importfile("C:\Users\Andrei\Documents\javelin-telemetry-system\tests\logs\FirsrLogs\861786.csv", [1, Inf]);
%
%  See also READTABLE.
%
% Auto-generated by MATLAB on 03-Apr-2025 09:57:52

%% Input handling

% If dataLines is not specified, define defaults
if nargin < 2
    dataLines = [1, Inf];
end

%% Set up the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 9);

% Specify range and delimiter
opts.DataLines = dataLines;
opts.Delimiter = ",";

% Specify column names and types
opts.VariableNames = ["accx", "accy", "accz", "gyrox", "gyroy", "gyroz", "magx", "magy", "magz"];
opts.VariableTypes = ["double", "double", "double", "double", "double", "double", "double", "double", "double"];

% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Import the data
data = readtable(filename, opts);

end