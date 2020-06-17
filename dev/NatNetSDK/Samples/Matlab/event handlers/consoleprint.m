function consoleprint( ~ , evnt )
	for i = 1:76
		fprintf( '\b' )
	end
	fprintf( '\n' )

	% the input argument, evnt, is a c structure half converted into a
	% matlab structure.
	data = evnt.data;
	rbnum = 1;

	% get the quaternion values of the rigid body and convert to Euler
	% Angles, right handed, global, XYZ order. Should be the same output
	% from Motive.
	q = quaternion( data.RigidBodies( rbnum ).qx, data.RigidBodies( rbnum ).qy, data.RigidBodies( rbnum ).qz, data.RigidBodies( rbnum ).qw );
	qRot = quaternion( 0, 0, 0, 1);
	q = mtimes( q, qRot);
	a = EulerAngles( q , 'zyx' );
	eulerx = a( 1 ) * -180.0 / pi;
	eulery = a( 2 ) * -180.0 / pi;
	eulerz = a( 3 ) * 180.0 / pi;
	
	% print the data to the command window
	fprintf( 'Frame:%5d ' , evnt.data.iFrame )

	fprintf( 'X:%0.1fmm ', data.RigidBodies( rbnum ).x * 1000 )
	fprintf( 'Y:%0.1fmm ', data.RigidBodies( rbnum ).y * 1000 )
	fprintf( 'Z:%0.1fmm ', data.RigidBodies( rbnum ).z * 1000 )
	
	fprintf( 'Pitch:%0.1f ', eulerx )
	fprintf( 'Yaw:%0.1f ', eulery )
	fprintf( 'Roll:%0.1f ', eulerz )

end
