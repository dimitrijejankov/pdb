

% Recursively traverse the query graph:
bottomUpSearch(Node, LeavesRemaining, ListofVisitedNodes):-

	% First we want to check:
	% If the node has been visited before?

	(member(Node, ListofVisitedNodes) ->

		% This "Node" has been visited before.

		% Check if any leaf node is still unexplored.
		% Start search from next leaf node.
		% Else we are done.

		length(LeavesRemaining, LengthList),
		(LengthList \= 0 ->
			[L|Rest] = LeavesRemaining,
			bottomUpSearch(L, Rest, ListofVisitedNodes)
			;
			true
		)
		;

		% The node has not been visited before.

		% Add this node to the list of visited.
		append(ListofVisitedNodes, [Node], Visited),


		% Are we at root node?
		(Node = virtualRootNode ->

			% Start from next leaf node.
			length(LeavesRemaining, LengthList),
			(LengthList \= 0 ->
				[L|Rest] = LeavesRemaining,
				bottomUpSearch(L, Rest, Visited)
				;
				true
			)
			;

			% Else go up and explore the next compute node:

			node(Node,Y,CompName,KeyValueList),

			% Scan Node:
			(Y = scan ->
				write(tcapOutput,Node),
				link(_,Node,Out,_,_),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= SCAN("),
				scan(Node,SetName,DBName),
				printString(SetName),write(tcapOutput,","),
				printString(DBName),write(tcapOutput,","),
				printString(CompName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),



				% There might be multiple children:
				findall(Child,link(Child,Node,_,_,_),Children),
				Children = [Next|Siblings],
				append(LeavesRemaining, Siblings, NewLeavesRemaining),
				link(Next,Node,_,_,_),
				bottomUpSearch(Next, NewLeavesRemaining, Visited)
			 ; true
			),



			% Apply Node:
			(Y = apply ->
				link(Node,_,_,Input, Projection),
				link(Next,Node,Out,_,_),
				apply(Node, InName, PrName, LName),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= APPLY("),
				printNameListPair(InName, Input), write(tcapOutput,","),
				printNameListPair(PrName, Projection), write(tcapOutput,","),
				printString(CompName), write(tcapOutput,","),
				printString(LName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			 ; true
			 ),


			% HashLeft Node:
			(Y = hashleft ->
				link(Node,_,_,Input, Projection),
				link(Next,Node,Out,_,_),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= HASHLEFT("),
				hashleft(Node, InName, PrName, LName),
				printNameListPair(InName, Input), write(tcapOutput,","),
				printNameListPair(PrName, Projection), write(tcapOutput,","),
				printString(CompName), write(tcapOutput,","),
				printString(LName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			 ; true
			 ),


			% HashRight Node:
			(Y = hashright ->
				link(Node,_,_,Input, Projection),
				link(Next,Node,Out,_,_),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= HASHRIGHT("),
				hashright(Node, InName, PrName, LName),
				printNameListPair(InName, Input), write(tcapOutput,","),
				printNameListPair(PrName, Projection), write(tcapOutput,","),
				printString(CompName), write(tcapOutput,","),
				printString(LName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			 ; true
			 ),


			% Join Node:
			(Y = join ->
				link(Next,Node,Out,_,_),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= JOIN("),
				join(Node, LeftInName, LeftPrName, RightInName, RightPrName),
				link(Node, LeftInName, _, LeftIn, LeftProject),
				link(Node, RightInName, _, RightIn, RightProject),
				printNameListPair(LeftInName, LeftIn), write(tcapOutput,","),
				printNameListPair(LeftPrName, LeftProject), write(tcapOutput,","),
				printNameListPair(RightInName, RightIn), write(tcapOutput,","),
				printNameListPair(RightPrName, RightProject), write(tcapOutput,","),
				printString(CompName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			; true
			),


			% Filter Node:
			(Y = filter ->
				link(Node,_,_,Input, Projection),
				link(Next,Node,Out,_,_),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= FILTER("),
				filter(Node, InName, PrName),
				printNameListPair(InName, Input), write(tcapOutput,","),
				printNameListPair(PrName, Projection), write(tcapOutput,","),
				printString(CompName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			; true
			),


			% Hashone Node:
			(Y = hashone ->
				link(Node,_,_,Input, Projection),
				link(Next,Node,Out,_,_),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= HASHONE("),
				hashone(Node, InName, PrName),
				printNameListPair(InName, Input), write(tcapOutput,","),
				printNameListPair(PrName, Projection), write(tcapOutput,","),
				printString(CompName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			; true
			),


			% Flatten Node:
			(Y = flatten ->
				link(Node,_,_,Input, Projection),
				link(Next,Node,Out,_,_),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= FLATTEN("),
				flatten(Node, InName, PrName),
				printNameListPair(InName, Input), write(tcapOutput,","),
				printNameListPair(PrName, Projection), write(tcapOutput,","),
				printString(CompName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			; true
			),


			% Output Node:
			(Y = output ->
				link(Node,_,_,Input, Projection),
				link(Next,Node,Out,_,_),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= OUTPUT("),
				output(Node, InName, SetName,DBName),
				printNameListPair(InName, Input), write(tcapOutput,","),
				printString(SetName),write(tcapOutput,","),
				printString(DBName),write(tcapOutput,","),
				printString(CompName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			 ; true
			),


			% Aggregate Node:
			(Y = aggregate ->
				link(Node,_,_,Input, Projection),
				link(Next,Node,Out,_,_),
				write(tcapOutput,Node),
				write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= AGGREGATE("),
				aggregate(Node, InName),
				printNameListPair(InName, Input), write(tcapOutput,","),
				printString(CompName),
				write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
				write(tcapOutput, ")\n"),
				bottomUpSearch(Next, LeavesRemaining, Visited)
			; true
			),


            % To support the new PARTITION computation in TCAP:
            % Partition Node:
            (Y = partition ->
                link(Node,_,_,Input, Projection),
                link(Next,Node,Out,_,_),
                write(tcapOutput,Node),
                write(tcapOutput,"("),printList(Out),write(tcapOutput,")<= PARTITION("),
                partition(Node, InName),
                printNameListPair(InName, Input), write(tcapOutput,","),
                printString(CompName),
                write(tcapOutput,", ["), printKeyValueList(KeyValueList), write(tcapOutput,"]"),
                write(tcapOutput, ")\n"),
                bottomUpSearch(Next, LeavesRemaining, Visited)
            ; true
            )
		)
	).





% Find the leaves:
findLeaves(Leaves):-
	findall(X0, scan(X0,_,_), Leaves).


% TCAP generator:
tcapGenerator():-
    open('/tmp/tcapOutput.tcap', write, ID, [alias(tcapOutput)]),
	findLeaves([H|T]),
	bottomUpSearch(H, T, []),
	close(ID).


% Print name-list pair: TCAP:
printNameListPair(ListName, List):-
	write(tcapOutput,ListName), write(tcapOutput,"("), printList(List), write(tcapOutput,")").


% Print string to console: 'String'
printString(String):-
	write(tcapOutput,"'"),write(tcapOutput,String),write(tcapOutput,"'").

% Print list to console:
printList([]).
printList([H|T]):-
	write(tcapOutput,H),
	length(T, Len),
	(	Len \= 0 -> write(tcapOutput,",")
		; true
	),
	printList(T).



% Print KeyValueList to console:
printKeyValueList([]).
printKeyValueList([H|T]):-
	H = (A,B),
	write(tcapOutput,"("), printString(A),write(tcapOutput,","),printString(B),write(tcapOutput,")"),
	length(T, Len),
	(	Len \= 0 -> write(tcapOutput,",")
		; true
	),
	printKeyValueList(T).