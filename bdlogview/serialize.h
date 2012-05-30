#pragma once

class serializer
{

};

class serialable
{
public:
	bool load(serializer* s) = 0;
	bool save(serializer* s) const = 0;
};
